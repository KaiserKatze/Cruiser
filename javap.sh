#!/bin/sh

path_rt=$JAVA_HOME/jre/lib/rt.jar
path_tmp=$HOME/tmp/runtime

parse()
{
    javap -v $1 > $1.log;
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
