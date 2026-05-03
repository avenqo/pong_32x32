## Software

This section describes individual software components and special features that need to be known for implementation.

### Sounds

All sounds are played from an SD card using the **DFPlayer Mini**. 
To avoid licensing issues, these MP3 files are not part of this project. Therefore, the sounds must be provided on a SD card in conjunction with the DFPlayer Mini. 

It's important to know that the DF Player plays the sounds by number.

You must therefore 
* create a collection for your sounds on SD card
* decide, what sound should be play for what event (Start Game, Goal, Start Round etc.)
* adapt in source code the number of the sound to be played accordingly

Here is an example of a section in DFPlayer.cpp of the software code that may need to be adjusted:

    ...
    void DFPlayer::startRound() {
    	player->play(38); // adapth this number
	}
	...


