-- Add music files to the playlist
-- (remember to use normal slashes "/", not backslashes "\"!)
AddToPlaylist("music/*.*");
--AddToPlaylist("C:/My Music/*.mp3");

-- Shuffle the playlist
ShufflePlaylist();

-- Play the first song from the playlist
-- The list keeps playing on repeat after this point. Press F1 in game to skip a track.
PlayMusic(GetNextSong());
