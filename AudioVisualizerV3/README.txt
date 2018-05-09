    _   _   _  ___  ___  ___   __   __ ___  ___  _   _   _    _     ___  ____ ___  ___ 
   /_\ | | | ||   \|_ _|/ _ \  \ \ / /|_ _|/ __|| | | | /_\  | |   |_ _||_  /| __|| _ \
  / _ \| |_| || |) || || (_) |  \ V /  | | \__ \| |_| |/ _ \ | |__  | |  / / | _| |   /
 /_/ \_\\___/ |___/|___|\___/    \_/  |___||___/ \___//_/ \_\|____||___|/___||___||_|_\
                                                                                       
V3.0
Released 5/9/18
Authors:
	Jake Carter
	Andrew Teta

The visualizer is broken up into component folders:
	CodeComposer:  	The folder containing the code for the MSP432
	Processing: 	The folder containing all the processing files that generate the visuals

The MSP analyzes audio data from a 3.5mm aux input and sends the signal's fourier components over UART to the Processing window, which reads the data from the UART buffer
and generates visuals.
