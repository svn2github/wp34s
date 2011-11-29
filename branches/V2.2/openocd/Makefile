# **********************************************************************************************
# FLASH	PROGRAMMING
#
# Alternate make target	for flash programming only
#
# You must create a special Eclipse make target	(program) to run this part of the makefile
# (Project -> Create Make Target... then set the Target	Name and Make Target to	"program")
#
# OpenOCD is run in "batch" mode with a	special	configuration file and a script	file containing
# the flash commands. When flash programming completes,	OpenOCD	terminates.
#
# Note that the	script file of flash commands (script.ocd) is part of the project
#
# Programmers: Martin Thomas, Joseph M Dupre, James P Lynch
# **********************************************************************************************

# specify output filename here (must be	*.bin file)
TARGET = ../realbuild/calc.bin

# specify the directory	where openocd executable resides
# We are using the Olimex install here
OPENOCD_DIR = $(ProgramFiles)\\OlimexODS\\OpenOCD\\
#OPENOCD	= $(OPENOCD_DIR)openocd-libftdi.exe
OPENOCD	= $(OPENOCD_DIR)bin\\openocd-0.5.0.exe

# specify OpenOCD configuration	file
OPENOCD_SERVER_CFG = at91sam7l128-server.cfg
OPENOCD_FLASH_CFG = at91sam7l128-program-flash.cfg

# program the AT91SAM7L128 internal flash memory
program: $(TARGET)
	@echo "Flash Programming with OpenOCD..." # display a message on the console
	$(OPENOCD) -f $(OPENOCD_FLASH_CFG) # program the onchip FLASH here
	@echo "Flash Programming Finished." # display a	message	on the console

# start	the server for debugging
server:
	@echo "Starting OpenOCD..." # display a	message	on the console
	$(OPENOCD) -f $(OPENOCD_SERVER_CFG) # Start the	server
	@echo "OpenOCD terminated." # display a	message	on the console
	

