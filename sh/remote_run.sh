#!/bin/sh

#getopts parsing
OPTIND=1         # Reset in case getopts has been used previously in the shell.
# Initialize our own variables:
label=""
global_path="/spool/users/boroden/remote/_default"

show_help() {
	echo "Usage: remote_run.sh -<options>"
	echo "-h - print help;"
	echo "-l - set label for data files;"
	echo "-p - set path to read from and write to."
	echo "Use -- to stop parsing parameters."
}
while getopts "h?l:p:" opt; do
	case "$opt" in
	h|\?)
		show_help
		exit 0
		;;
	l)	label=$OPTARG
		;;
	p)	global_path=$OPTARG
		;;
	esac
done

shift $((OPTIND-1))
[ "$1" = "--" ] && shift

sim_label="";
for el in $@; do
	if [[ $el == "--sim" ]]; then
		sim_label="_sim"
	fi
done

echo "label='$label', global_path='$global_path', options ignored: $@, sim_label='$sim_label'"
#getopts parsing end

#log files should have diff label for simulation

echo "qsub call:"
qsub -q remote -soft -l time=25:00:00 -m bea -M aurthem@gmail.com \
	-wd /spool/users/boroden/current/KdRecTtau -S /bin/bash \
	-e $global_path/remote${label}${sim_label}.err -o $global_path/remote${label}${sim_label}.log \
	/spool/users/boroden/current/KdRecTtau/sh/run.sh --remote --path \
	$global_path --label $label $@
echo "done"
#	-t 7883-7893:1 \
#	-e /home/boroden/current/log/remote-$JOB_NAME-$JOB_ID-$TASK_ID.err \
#	-o /home/boroden/current/log/remote-$JOB_NAME-$JOB_ID-$GE_TASK_ID.log

#	-verify
#-w ewpv
# -t n-m:s	n=GE_TASK_FIRST, m=GE_TASK_LAST, s=GE_TASK_STEPSIZE

#to check/delete job:
#qstat -t
#qdel <job_id>
