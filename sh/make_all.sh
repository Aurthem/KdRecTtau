CURRENT_PROGRAM_PATH=/spool/users/boroden/current/KdRecTtau
CURRENT_DATACLASS_PATH="$CURRENT_PROGRAM_PATH"/DataClass

#call clean to delete previous
if [ "$1" = "clean" ]; then
	echo "Cleaning up"
	make -C "$CURRENT_DATACLASS_PATH" -w clean
	make -C "$CURRENT_PROGRAM_PATH" -w clean
else
	echo "Compiling"
	make -C "$CURRENT_DATACLASS_PATH" -w
	make -C "$CURRENT_PROGRAM_PATH" -w
fi

#cd ~/current/KdRecTtau && cd DataClass && make clean && make && cd .. && make clean && make
