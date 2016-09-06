# WSTF_Sniffer
WSTF_Sniffer
*********BASIC DEVELOPING INSTRUCTIONS************
1) You need to have a GIT BASH shell installed or some other thing maybe Hitch can expand on
2) in order to access the robot while you are here you need an ssh tool installed, simply entering 'ssh' and executing should be 
   enough to check, also check for nano text editor again 'nano' then the file name you want to open will confirm this
3) Now 'cd' into the desired directory if you do not know the subfolder name use 'ls' (visible files) or 'ls -a' (all files), 
   which for us is ../Google Drive/...../Source/WSTF_Sniffer_Git/..
4) Now you are in the directory that has your file you can now edit, compile, and run git commands
     a) to edit 'nano filename' different commands within the text editor are shown at the bottom
     b) to compile your code simply hit 'make filename' or simply 'make', it is also good practice to enter 'make clean' before 
        each compile. 
     c) Now that your code is error free and ready to share, to commit it to Git: 'git commit' look to see if the file you desire 
        to commit is "staged for commit"
            if not, try 'git add' then 'git commit' again. Now write something then [ctrl][x] to save, now enter 'git push'.
To ensure you have the most recent changes 'git fetch' then 'git pull' should update your current directories -so- not a good 
idea in the middle of editing a file, you will most likely lose it.
