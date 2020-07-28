#!/bin/bash

usage()
{
    echo "usage: [[-gen baseDir mddName headerName] | [-h]]"
}

gen()
{
    python3 gen_map.py $baseDir $mddName
    # cmake CMakeLists.txt
    make clean
    make
    ./bert_gen $baseDir $headerName
}

##### Main
if [ $# -eq 0 ]
then
    usage
fi

while test $# -gt 0; do
    case $1 in
        -h | --help )
            usage
            shift
            ;;
        
        -gen )
            shift
            baseDir=$1
            shift
            mddName=$1
            shift
            headerName=$1
            shift
            gen
            ;;

        * )
            break
            ;;
    esac
done
        