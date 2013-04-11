#!/bin/bash

#gnome-terminal -x ~/.profile & cd eslipse

#xterm -hold -e "cd ~/eclipse/f4/ && . ~/.profile"

#xterm -hold -e "cd ~/eclipse/f4/  && ls"

#gnome-terminal -x  "cd ~/eclipse/f4/  && ls"

#gnome-terminal -e "bash -c \"cd eclipse/f4; echo $PATH;. ~/.profile; echo $PATH; exec bash\"

#gnome-terminal -e "bash -c \"echo ${PWD##*/}; cd ~/code/OpenPilot/; . ~/.profile; exec bash\""

gnome-terminal -e "bash -c \"cd ${PWD##*/}; . ~/.profile; exec bash\""
