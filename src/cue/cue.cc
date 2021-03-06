/*
 * Cue Sheet Plugin for Audacious
 * Copyright (c) 2009-2015 William Pitcock and John Lindgren
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions, and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions, and the following disclaimer in the documentation
 *    provided with the distribution.
 *
 * This software is provided "as is" and without any warranty, express or
 * implied. In no event shall the authors be liable for any damages arising from
 * the use of this software.
 */

#include <string.h>

extern "C" {
#include <libcue/libcue.h>
}

#include <libaudcore/audstrings.h>
#include <libaudcore/i18n.h>
#include <libaudcore/plugin.h>
#include <libaudcore/probe.h>

static const char * const cue_exts[] = {"cue"};

class CueLoader : public PlaylistPlugin
{
public:
    static constexpr PluginInfo info = {N_("Cue Sheet Plugin"), PACKAGE};
    constexpr CueLoader () : PlaylistPlugin (info, cue_exts, false) {}

    bool load (const char * filename, VFSFile & file, String & title,
     Index<PlaylistAddItem> & items);
};

EXPORT CueLoader aud_plugin_instance;

bool CueLoader::load (const char * cue_filename, VFSFile & file, String & title,
 Index<PlaylistAddItem> & items)
{
    Index<char> buffer = file.read_all ();
    if (! buffer.len ())
        return false;

    buffer.append (0);  /* null-terminate */

    Cd * cd = cue_parse_string (buffer.begin ());
    int tracks = cd ? cd_get_ntrack (cd) : 0;

    if (tracks < 1)
        return false;

    Track * cur = cd_get_track (cd, 1);
    const char * cur_name = cur ? track_get_filename (cur) : nullptr;

    if (! cur_name)
        return false;

    bool new_file = true;
    String filename;
    PluginHandle * decoder = nullptr;
    Tuple base_tuple;

    for (int track = 1; track <= tracks; track ++)
    {
        if (new_file)
        {
            filename = String (uri_construct (cur_name, cue_filename));
            decoder = filename ? aud_file_find_decoder (filename, false) : nullptr;
            base_tuple = decoder ? aud_file_read_tuple (filename, decoder) : Tuple ();
        }

        Track * next = (track + 1 <= tracks) ? cd_get_track (cd, track + 1) : nullptr;
        const char * next_name = next ? track_get_filename (next) : nullptr;

        new_file = (! next_name || strcmp (next_name, cur_name));

        if (base_tuple)
        {
            Tuple tuple = base_tuple.ref ();
            tuple.set_filename (filename);
            tuple.set_int (Tuple::Track, track);

            int begin = (int64_t) track_get_start (cur) * 1000 / 75;
            tuple.set_int (Tuple::StartTime, begin);

            if (new_file)
            {
                int length = base_tuple.get_int (Tuple::Length);
                if (length > 0)
                    tuple.set_int (Tuple::Length, length - begin);
            }
            else
            {
                int length = (int64_t) track_get_length (cur) * 1000 / 75;
                tuple.set_int (Tuple::Length, length);
                tuple.set_int (Tuple::EndTime, begin + length);
            }

            Cdtext * cdtext = track_get_cdtext (cur);

            if (cdtext)
            {
                const char * s;
                if ((s = cdtext_get (PTI_PERFORMER, cdtext)))
                    tuple.set_str (Tuple::Artist, s);
                if ((s = cdtext_get (PTI_TITLE, cdtext)))
                    tuple.set_str (Tuple::Title, s);
            }

            items.append (String (filename), std::move (tuple), decoder);
        }

        if (! next_name)
            break;

        cur = next;
        cur_name = next_name;
    }

    return true;
}
