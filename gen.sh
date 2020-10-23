#!/bin/bash

curr="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/$(basename "${BASH_SOURCE[0]}")"
bertDir="$( cd "$( dirname $curr )" && pwd )"

usage() {
    echo "usage: gen.sh [[-gen dcpFile headerName | [-h] | [-gen_tcl dcpFile headerName] | [-map_gen wkDir mddName] | [-header_gen wkDir]]"
}

header_gen() {
    # cmake CMakeLists.txt
    #make clean
    #make
    $bertDir/bert_gen/bert_gen $wkDir ${headerName}_uncompressed
    cp $bertDir/bert_gen/compress/compress_generic.c $wkDir
    cp $bertDir/bert_gen/compress/ultrascale_plus.? $wkDir
    cp $bertDir/bert_gen/compress/bert_types.h $wkDir
    cd $wkDir
    echo "#include \"${headerName}_uncompressed.h\"" >${headerName}_compress.c
    echo "#include \"stdio.h\"" >>${headerName}_compress.c
    echo "#include \"compress_generic.c\"" >>${headerName}_compress.c
    gcc -c ${headerName}_compress.c
    gcc -c ultrascale_plus.c
    gcc -c ${headerName}_uncompressed.c
    gcc -o ${headerName}_ucompress ${headerName}_compress.o ${headerName}_uncompressed.o ultrascale_plus.o
    ./${headerName}_ucompress >${headerName}.c
    cp ${headerName}_uncompressed.h ${headerName}.h
    rm ultrascale_plus.o ${headerName}_compress.o ${headerName}_uncompressed.o ${headerName}_ucompress

}

map_gen() {
    python3 $bertDir/bert_gen/gen_map.py $wkDir $mddName
}

call_vivado() {
    vivado -mode batch -source $1
}

tcl_gen() {
    rm $2/*.tcl
    echo "open_checkpoint $1" >$2/bert.tcl
    echo "source $wkDir/mdd_make.tcl" >>$2/bert.tcl
    echo "mddMake $wkDir/$header_name" >>$2/bert.tcl
    echo "write_bitstream -logic_location_file $2/$header_name" >>$2/bert.tcl

    cp $bertDir mdd_make.tcl $wkDir
}

##### Main
if [ $# -eq 0 ]; then
    usage
fi

while test $# -gt 0; do
    case $1 in
    -h | --help)
        usage
        shift
        ;;

    -gen)
        if [ $# -gt 2 ]; then
            shift
            dcp=$1
            header_name=$2
            baseDir=dirname $dcp
            $(mkdir $baseDir/bert_src)
            wkDir=$baseDir/bert_src
            tcl_gen $dcp $wkDir
            call_vivado $wkDir/bert.tcl
            mddName=$header_name.mdd
            map_gen
            header_gen
        else
            usage
            exit
        fi
        ;;

    -gen_tcl)
        if [ $# -gt 2 ]; then
            shift
            dcp="$(cd "$(dirname "${BASH_SOURCE[1]}")" && pwd)/$(basename "${BASH_SOURCE[1]}")"
            header_name=$2
            baseDir="$( cd "$( dirname $dcp )" && pwd )"
            mkdir -p $baseDir/bert_src
            wkDir=$baseDir/bert_src
            echo $wkDir
            tcl_gen $dcp $wkDir
            shift
            exit
        else
            usage
            exit
        fi
        ;;

    -map_gen)
        if [ $# -gt 2 ]; then
            shift
            wkDir=$1
            mddName=$2
            map_gen
        else
            usage
            exit
        fi
        ;;

    -header_gen)
        if [ $# -gt 1 ]; then
            shift
            wkDir = $1
            header_gen
        else
            usage
            exit
        fi
        ;;

    *)
        echo "Illegal operation"
        usage
        exit
        ;;
    esac
done
