#!/bin/sh

if [ -z $JAVA_HOME ]
then
    echo "Environment variable JAVA_HOME is null!"
fi

path_rt=$JAVA_HOME/jre/lib/rt.jar
path_tmp=$HOME/tmp/runtime
alias command='javap -v -p -s -constants -c -l'

parse()
{
    command $1 > $1.log;
}

check()
{
    local dir=$1;
    for entry in `ls $dir`
    do
        entry=$dir/$entry;
        if [ -f $entry ]
        then
            parse $entry;
        fi
        if [ -d $entry ]
        then
            check $entry;
        fi
    done
}

if [ -z `which javap` ]
then
    echo "Fail to locate javap!";
fi

if [ ! -f $path_rt ]
then
    echo "Fail to locate rt.jar!";
fi

if [ ! -d $path_tmp ]
then
    mkdir -p $path_tmp;
    cd $path_tmp;
    unzip $path_rt;
fi

cd $path_tmp;

check $path_tmp;
