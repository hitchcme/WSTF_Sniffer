#!/bin/bash

echo -e "Running apt-get, with --assume-yes option to answer yes, to: Are you sure questions"
echo -e "\n"
echo "apt-get --assume-yes install libgps-dev"
#apt-get --assume-yes install libgps-dev

echo -e "\n"
echo -e "to compile the example, run: 		Note: Dont just copy and past this.  READ & UNDERSTAND it."
echo -e "						FILENAME = The File you want to compile"
echo -e "						NAME_OF_OUTPUT_FILE = The name you want to give the end resulting executable"
echo -e "						at this point, you should also know not to use the cash symbols, unless youre"
echo -e "						calling a defined variable, in this shell..."
echo -e "						Pardon my crankiness...\n\n"
echo -e "						Enough of my ranting, just run the shit!!!\n\n"
echo -e "\t\t gcc \$FILENAME -lm -lgps -o \$NAME_OF_OUTPUT_FILE"
echo -e "\n\n"
echo -e "to run the newly created executable:\n\n"
echo -e "\t\t ./\$NAME_OF_OUTPUT_FILE"
echo -e "\n\n"
