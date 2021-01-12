#!/bin/bash

forbidden_global_header_files="CAGE QBContentManager QBDataModel3 QBDLNAClient QBFSReader QBPlatformHAL QBSearch SvEPG SvDataBucket2 SvCore SvEPGDataLayer SvFoundation SvPlayerKit SvXMLRPCClient swl XMB2 main"

# prepare forbidden_global_header_files for grep use
forbidden_global_header_files=$(echo $forbidden_global_header_files | sed 's_ _\\\|_g')

function open_file
{
	echo -en "\t\t\t<div><h4>$1</h4>\n"
}

function open_section
{
	echo -en "\t\t\t\t<div><h5>$1</h5>\n<pre>\n"
}

function close_section
{
	echo -en "</pre></div>\n"
}

function close_file
{
	echo -en "\t\t\t</div>\n"
}

function html_escape
{
	sed -e 's/&/\&amp;/g' -e 's/</\&lt;/g' -e 's/>/\&gt;/g' -e 's/"/\&quot;/g'
}

function check_header_file
{
	hdr_file=$(echo $1 | sed -e 's;//*;/;' -e 's;^\./;;')
	test_src_file_base=$2
	problems_count=0

    if [ $CHECK_FORBIDDEN_GLOBAL_HEADERS -gt 0 ]; then
        # check if forbidden global header files are included
        res=$( grep -nH "^[[:space:]]*#include[[:space:]]*[<\"]\(${forbidden_global_header_files}\)\.h[>\"]" ${hdr_file})
        if [ $? -eq 0 ]; then
            [ ${problems_count} -eq 0 ] && open_file ${hdr_file}
            open_section 'forbidden <tt>#include</tt> directive'
            echo "$res" | html_escape
            close_section
            problems_count=$((problems_count + 1))
        fi
    fi # CHECK_FORBIDDEN_GLOBAL_HEADERS

    if [ $CHECK_HEADERS_WITH_COMPILER -gt 0 ]; then
        hdr_file_ext=$(echo $hdr_file | sed 's/^.*\(\.[^.]*\)$/\1/g')
        if [ "f${hdr_file_ext}" == "f.h" ]; then
            compiler=cross-gcc
            test_src_file=${test_src_file_base}.c
        elif [ "f${hdr_file_ext}" == "f.hpp" ] || [ "f${hdr_file_ext}" == "f.hh" ]; then
            compiler=cross-g++
            test_src_file=${test_src_file_base}.cc
        else
            [ ${problems_count} -gt 0 ] && close_file
            return ${problems_count}
        fi

        ${compiler} ${CFLAGS} -include ${hdr_file} -o ${test_src_file_base}.o -c ${test_src_file} &>/dev/null
        if [ $? -ne 0 ]; then
            [ ${problems_count} -eq 0 ] && open_file ${hdr_file}
                open_section "header file won't compile"
            ${compiler} ${CFLAGS} -include ${hdr_file} -o ${test_src_file_base}.o -c ${test_src_file} 2>&1 | html_escape
            close_section
            problems_count=$((problems_count + 1))
        fi
    fi # CHECK_HEADERS_WITH_COMPILER

    if [ $CHECK_LICENSE -gt 0 ]; then
        # check license
        res=$(head -n 1 ${hdr_file} | grep '^\/\*')
        if [ $? -eq 1 ]; then
                [ ${problems_count} -eq 0 ] && open_file ${hdr_file}
                open_section 'software license header not found'
                echo "source files must start with a license header from /Var/qblicense" | html_escape
                close_section
                problems_count=$((problems_count + 1))
        fi
    fi # CHECK_LICENSE

    if [ $CHECK_INCLUDE_GUARDS -gt 0 ]; then
        res=$(grep '^#ifndef\|^#endif' ${hdr_file} | wc -l)
        if [ ${res} -lt 2 ]; then
                [ ${problems_count} -eq 0 ] && open_file ${hdr_file}
                open_section 'include guards not found'
                echo "header file should have include guards" | html_escape
                close_section
                problems_count=$((problems_count + 1))
        fi
    fi # CHECK_INCLUDE_GUARDS

    if [ $CHECK_WHITESPACE_CHARACTERS -gt 0 ]; then
        res=$( grep -nHP "\t|[[:space:]]+$" ${hdr_file})
        if [ $? -eq 0 ]; then
            [ ${problems_count} -eq 0 ] && open_file ${hdr_file}
            open_section 'forbidden <tt>whitespace characters found</tt>'
            echo "$res" | html_escape
            close_section
            problems_count=$((problems_count + 1))
        fi
    fi # CHECK_WHITESPACE_CHARACTERS

	[ ${problems_count} -gt 0 ] && close_file

	return ${problems_count}
}

#######################################################################

function fail
{
	echo "check_header_sanity: $1" >&2
	exit 1
}

function import_config
{
    if [ -z "$SANITY_CHECKER_CONFIG" ]; then
        SANITY_CHECKER_CONFIG=lenient
    fi

    CONF_FILE=${SRM_ROOT}/host/local/usr/local/share/sanity_checker/${SANITY_CHECKER_CONFIG}
    if [ -f "$CONF_FILE" ]; then
        source $CONF_FILE
    else
        fail "config file '$SANITY_CHECKER_CONFIG' not found"
    fi
}


import_config

component_name="__empty__"
tmp_dir=""
includes=""
any_error_found=false

# parse command line
while [ $# -gt 0 ]; do
	if [ $1 == "-t" ]; then
		[ $# -eq 1 ] && fail 'invalid usage'
		tmp_dir="$2"
		shift 2
	elif [ $1 == "-c" ]; then
		[ $# -eq 1 ] && fail 'invalid usage'
		# sanitize component name
		component_name=$(echo $2 | sed -e 's/_/__/' -e 's;/;_;')
		shift 2
	else
		includes="$includes $1"
		shift
	fi
done

# find include dir
include_dir="${SRM_ROOT}/host/cross/${CROSS_ARCH}/usr/local/include"
[ -d ${include_dir} ] || fail "cannot find include dir in sbuild ${SRM_ROOT}"

# find header files
header_files=''
for i in $includes; do
	if [ ${i/%\.h/} != $i ]; then
		if [ -f $i ]; then
			header_files="$header_files $i"
		elif [ -f $include_dir/$i ]; then
			header_files="$header_files $include_dir/$i"
		else
			header_files="$header_files $(find $include_dir -type f -a -name $i 2>/dev/null)"
		fi
	else
		if [ -d $i ]; then
			header_files="$header_files $(find $i -type f -a -name '*.h' 2>/dev/null)"
		elif [ -d $include_dir/$i ]; then
			header_files="$header_files $(find $include_dir/$i -type f -a -name '*.h' 2>/dev/null)"
		else
			dir=$(find $include_dir -type d -a -name $i 2>/dev/null)
			[ ! -z "$dir" ] && header_files="$header_files $(find $dir -type f -a -name '*.h' 2>/dev/null)"
		fi
	fi
done

if [ -z "$header_files" ]; then
	# no input files, do noting
	exit 0
fi

# create temporary directory
if [ -z "$tmp_dir" ]; then
	tmp_dir=$(mktemp -d -t header_sanity_check.XXXXXX)
else
	[ ${tmp_dir:0:1} == "/" ] || tmp_dir=$(pwd)/${tmp_dir}
	mkdir -p ${tmp_dir}
	tmp_dir=$(mktemp -d ${tmp_dir}/header_sanity_check.XXXXXX)
fi

# create test files
test_src_file_base=${tmp_dir}/test
echo -en "extern int _test_global_var_;\nint _test_global_var_ = 0;\n" >${test_src_file_base}.c
echo -en "extern int _test_global_var_;\nint _test_global_var_ = 0;\n" >${test_src_file_base}.cc

# generate output file
report_parts_dir=${SRM_ROOT}/docs/header_sanity_report
if [ ! -d ${report_parts_dir} ] && [ -d ${SRM_ROOT}/base ]; then
	base_root_dir="${SRM_ROOT}/base/sbuild-*"
	if [ -d ${base_root_dir}/docs/header_sanity_report ]; then
		cp -r ${base_root_dir}/docs/header_sanity_report ${report_parts_dir}
	fi
fi
mkdir -p ${report_parts_dir}
echo -n >${report_parts_dir}/___empty___
report_part_file=${report_parts_dir}/${component_name}
echo -n >${report_part_file}

# filter out '-include FILE' from CFLAGS
CFLAGS="$(echo $CFLAGS | sed -r 's/[[:space:]]*-include[[:space:]]+[^[:space:]]+[[:space:]]*/ /g')"
export CFLAGS

# do the checking
echo -en "\t\t<div><a name="${component_name}"/>\n" >>${report_part_file}
echo -en "\t\t\t<h2>Component ${component_name}</h2>\n" >>${report_part_file}
for f in ${header_files}; do
	check_header_file $f ${test_src_file_base} >>${report_part_file}
    if [ "$?" != "0" ]; then
        any_error_found=true
    fi
done
echo -en "\t\t</div>\n" >>${report_part_file}
if [ $(wc -l <${report_part_file}) -eq 3 ]; then
	# nothing found
	rm -f ${report_part_file}
fi

# generate report file
report_file=${SRM_ROOT}/docs/header_sanity_report.html
cat <<EOT >${report_file}
<html>
	<head>
		<title>Header files sanity report</title>
		<style type="text/css">
			body { font-family: Cantarell,Ubuntu,'Droid Sans','Vera Sans',sans-serif; font-size: 92%; color: #555; padding-left: 4px; }
			div { border: solid #5f5f5f 4px; border-radius: 10px; padding-left: 4px; margin: 2ex 10%; }
			div div { border: solid #5f5f5f; border-width: 0px 0 0px 0px; border-radius: 0; margin: 1ex 0.5em; }
			div div div { border: solid #5f5f5f; border-width: 0 0 0 2px; padding: 2px; background: #cfcfcf; }
			div div div * { margin: 0.5ex 0.25em; padding: 0; }
			h1, h2 { margin: 0.5ex 2em; text-align: center; }
			h2 { text-decoration: underline; }
			h3, h4 { margin: 0.5ex 0.25em; padding: 0; }
		</style>
	</head>
	<body>
		<h1>Header files sanity report</h1>
EOT
for part in $(ls -1 -t -r ${report_parts_dir}/*); do
	cat $part >>${report_file}
done
echo -en "\t</body>\n</html>\n" >>${report_file}

rm -rf ${tmp_dir}

if [ "$any_error_found" == true ]; then
    exit $EXIT_ON_ERROR
fi

exit 0
