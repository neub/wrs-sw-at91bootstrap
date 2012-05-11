#!/bin/bash

showhelp()
{
	echo "Usage: $0 [options]"
	echo "options:"
	echo "		--help: show this little help"
	echo "		--df: compile only for dataflash"
	echo "		--nf: compile only for nandflash"
}



case "$1" in
	--help) showhelp;; 
	--nf) 	yes "" | make at91sam9g45nf_defconfig > /dev/null; make;;
	--df) 	yes "" | make at91sam9g45df_defconfig > /dev/null; make;;
	*) 	yes "" | make at91sam9g45df_defconfig > /dev/null; make; yes "" | make at91sam9g45nf_defconfig > /dev/null; make;;
esac


