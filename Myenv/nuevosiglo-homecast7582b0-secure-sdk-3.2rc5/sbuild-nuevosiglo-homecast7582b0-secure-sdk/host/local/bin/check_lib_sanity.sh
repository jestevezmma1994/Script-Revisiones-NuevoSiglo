#!/bin/bash

# TODO: add SvWeakListGetIterator and SvWeakListGetReverseIterator to dangerous functions
dangerous_functions=" sprintf vsprintf strtof strtod atof mktemp tempnam tmpnam readdir strtok asctime ctime gmtime localtime dirname basename "
obsolete_functions=" SvInterfaceIsImplementedByObject "
nonportable_functions=" strchrnul getline gethostbyaddr_r gethostbyname_r gethostbyname2_r "
blocking_functions=" gethostbyaddr gethostbyaddr_r gethostbyname gethostbyname_r gethostbyname2 gethostbyname2_r getaddrinfo getnameinfo "

function open_section
{
	echo -en "\t\t<div><h3>$1</h3>\n\t\t\t<div><ul>\n"
}

function close_section
{
	echo -en "\t\t\t</ul></div>\n\t\t</div>\n"
}

function html_escape
{
	sed -e 's/&/\&amp;/g' -e 's/</\&lt;/g' -e 's/>/\&gt;/g' -e 's/"/\&quot;/g'
}

function find_function_declarations
{
	symbol_name=$1
	shift
	grep -E "^(|.*[^a-zA-Z0-9_])${symbol_name}(|[^a-zA-Z0-9_].*)$" $@ | wc -l
}

function check_symbols
{
	expf_problems_count=0
	expv_problems_count=0
	impf_problems_count=0
	impv_problems_count=0
	lib="$1"
	shift

	while read symbol_type symbol_name; do
		if [ $symbol_type == "expf" ]; then
			[ "$symbol_name" == "__gmon_start__" ] && continue
			if [ $# -gt 0 ]; then
				# check if exported functions are declared in any header file
				total_cnt=$(find_function_declarations $symbol_name $@)
				if [ $total_cnt -eq 0 ]; then
					[ ${expf_problems_count} -eq 0 ] && open_section 'library exports unknown functions'
					symbol_name=$(echo $symbol_name | c++filt | sed 's;\([^\)]\)$;\1();' | html_escape)
					echo -en "\t\t\t\t<li>library exports function <tt>${symbol_name}</tt> which is not declared in any header file</li>\n"
					expf_problems_count=$((expf_problems_count + 1))
				fi
			else
				#[ ${expf_problems_count} -eq 0 ] && open_section 'exported functions not checked, library header files unknown'
				#expf_problems_count=$((expf_problems_count + 1))
				true
			fi
		elif [ $symbol_type == "expv" ]; then
			if [ $expf_problems_count -gt 0 ]; then close_section; expf_problems_count=0; fi
			# report every exported global variable
			[ ${expv_problems_count} -eq 0 ] && open_section 'library exports global variables'
			symbol_name=$(echo $symbol_name | c++filt | html_escape)
			echo -en "\t\t\t\t<li>library exports global variable <tt>${symbol_name}</tt></li>\n"
			expv_problems_count=$((expv_problems_count + 1))
		elif [ $symbol_type == "impf" ]; then
			if [ $expf_problems_count -gt 0 ]; then close_section; expf_problems_count=0; fi
			if [ $expv_problems_count -gt 0 ]; then close_section; expv_problems_count=0; fi
            if [ $DANGEROUS_FUNCTIONS -gt 0 ]; then
                # check if external function used by this library is dangerous
                if [ "${dangerous_functions/ $symbol_name /}" != "$dangerous_functions" ]; then
                    [ ${impf_problems_count} -eq 0 ] && open_section 'library uses dangerous, obsolete or non-portable functions'
                    echo -en "\t\t\t\t<li>library uses dangerous function <tt>${symbol_name}()</tt></li>\n"
                    impf_problems_count=$((impf_problems_count + 1))
                fi
            fi #DANGEROUS_FUNCTIONS
            if [ $OBSOLETE_FUNCTIONS -gt 0 ]; then
                if [ "${obsolete_functions/ $symbol_name /}" != "$obsolete_functions" ]; then
                    [ ${impf_problems_count} -eq 0 ] && open_section 'library uses dangerous, obsolete or non-portable functions'
                    echo -en "\t\t\t\t<li>library uses obsolete function <tt>${symbol_name}()</tt></li>\n"
                    impf_problems_count=$((impf_problems_count + 1))
                fi
            fi # OBSOLETE_FUNCTIONS
            if [ $NONPORTABLE_FUNCTIONS -gt 0 ]; then
                if [ "${nonportable_functions/ $symbol_name /}" != "$nonportable_functions" ]; then
                    [ ${impf_problems_count} -eq 0 ] && open_section 'library uses dangerous, obsolete or non-portable functions'
                    echo -en "\t\t\t\t<li>library uses non-portable function <tt>${symbol_name}()</tt></li>\n"
                    impf_problems_count=$((impf_problems_count + 1))
                fi
            fi # NONPORTABLE_FUNCTIONS
            if [ $BLOCKING_FUNCTIONS -gt 0 ]; then
                if [ "${blocking_functions/ $symbol_name /}" != "$blocking_functions" ]; then
                    [ ${impf_problems_count} -eq 0 ] && open_section 'library uses dangerous, obsolete or non-portable functions'
                    echo -en "\t\t\t\t<li>library uses blocking function <tt>${symbol_name}()</tt></li>\n"
                    impf_problems_count=$((impf_problems_count + 1))
                fi
            fi # BLOCKING_FUNCTIONS
		elif [ $symbol_type == "impv" ]; then
			[ "$symbol_name" == "__fpscr_values" ] && continue
			if [ $expf_problems_count -gt 0 ]; then close_section; expf_problems_count=0; fi
			if [ $expv_problems_count -gt 0 ]; then close_section; expv_problems_count=0; fi
			if [ $impf_problems_count -gt 0 ]; then close_section; impf_problems_count=0; fi
			# report every external variable used by this library
			[ ${impv_problems_count} -eq 0 ] && open_section 'library uses external variables'
			echo -en "\t\t\t\t<li>library uses external variable <tt>${symbol_name}</tt></li>\n"
			impv_problems_count=$((impv_problems_count + 1))
		fi
	done
	[ $expf_problems_count -gt 0 ] && close_section && any_error_found=true
	[ $expv_problems_count -gt 0 ] && close_section && any_error_found=true
	[ $impf_problems_count -gt 0 ] && close_section && any_error_found=true
	[ $impv_problems_count -gt 0 ] && close_section && any_error_found=true
}

#######################################################################

function fail
{
	echo "check_lib_sanity: $1" >&2
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

any_error_found=false

# parse command line
[ $# -lt 1 ] && fail 'invalid usage'
lib="$1"
shift
[ ! -f "$lib" ] && fail "file $lib not found"
[ ${lib:0:1} == "/" ] || lib="$(pwd)/$lib"

component_name="$(echo $lib | sed 's;^.*/lib\([^\./]*\)\.[^/]*$;\1;')"
if [ -z "$component_name" ] && [ ! -z "$SRM_COMPONENT" ]; then
	component_name="$(basename ${SRM_COMPONENT})"
elif [ -z "$component_name" ]; then
	component_name="---unknown---"
fi

# check sbuild
sbuild_dir="$lib"
while [ ! -z "$sbuild_dir" ]; do
	if [ -d $sbuild_dir ]; then
		base=$(basename $sbuild_dir)
		[ ${base:0:7} == "sbuild-" ] && break
	fi
	sbuild_dir="$(echo $sbuild_dir | sed 's;/[^/]*$;;')"
done
[ -z "$sbuild_dir" ] && fail "file $lib not in sbuild"

# find include dir
cross_arch="$CROSS_ARCH"
[ -z "$CROSS_ARCH" ] && cross_arch="$(grep -o 'CROSS_ARCH=[^<]*' $sbuild_dir/environment.xml | tail -1 | cut -d= -f2)"
if [ -z "$cross_arch" ] || [ ! -d $sbuild_dir/host/cross/$cross_arch ]; then
	fail "cannot determine CROSS_ARCH of sbuild $sbuild_dir"
fi
include_dir="$sbuild_dir/host/cross/$cross_arch/usr/local/include"

# find include files
include_files=''
for i in $@; do
	if [ ${i/%\.h/} != $i ]; then
		if [ -f $i ]; then
			include_files="$include_files $i"
		elif [ -f $include_dir/$i ]; then
			include_files="$include_files $include_dir/$i"
		else
			include_files="$include_files $(find $include_dir -type f -a -name $i 2>/dev/null)"
		fi
	else
		if [ -d $i ]; then
			include_files="$include_files $(find $i -type f -a -name '*.h' 2>/dev/null)"
		elif [ -d $include_dir/$i ]; then
			include_files="$include_files $(find $include_dir/$i -type f -a -name '*.h' 2>/dev/null)"
		else
			dir=$(find $include_dir -type d -a -name $i 2>/dev/null)
			[ ! -z "$dir" ] && include_files="$include_files $(find $dir -type f -a -name '*.h' 2>/dev/null)"
		fi
	fi
done

# generate output file
report_parts_dir=$sbuild_dir/docs/libs_sanity_report
if [ ! -d ${report_parts_dir} ] && [ -d ${SRM_ROOT}/base ]; then
	base_root_dir="${SRM_ROOT}/base/sbuild-*"
	if [ -d ${base_root_dir}/docs/libs_sanity_report ]; then
		cp -r ${base_root_dir}/docs/libs_sanity_report ${report_parts_dir}
	fi
fi
mkdir -p ${report_parts_dir}
echo -n >${report_parts_dir}/___empty___
report_part_file=${report_parts_dir}/${component_name}
echo -n >${report_part_file}

# do the checking
echo -en "\t<div><a name=\"${component_name}\">\n" >>${report_part_file}
echo -en "\t\t<h2>Library $(basename $lib)</h2>\n" >>${report_part_file}
objdump -T $lib | grep -e '^[^[:space:]]* [^l].....[FO ]' | sed -e 's;^[^[:space:]]* ;;' |\
 gawk '/F[[:space:]]+.text[[:space:]]+/ { print "expf", $NF }
       /O[[:space:]]+.(ro|)data[[:space:]]+/ { print "expv", $NF }
       /F[[:space:]]+\*UND\*[[:space:]]+/ { print "impf", $NF }
       /O[[:space:]]+\*UND\*[[:space:]]+/ { print "impv", $NF }' |\
 sort | check_symbols $lib $include_files >>${report_part_file}
echo -en "\t</div>\n" >>${report_part_file}
if [ $(wc -l <${report_part_file}) -eq 3 ]; then
	# nothing found
	rm -f ${report_part_file}
fi

# generate report file
report_file=${sbuild_dir}/docs/libs_sanity_report.html
cat <<EOT >${report_file}
<html>
	<head>
		<title>Shared libraries sanity report</title>
		<style type="text/css">
			body { font-family: Cantarell,Ubuntu,'Droid Sans','Vera Sans',sans-serif; font-size: 92%; color: #555; padding-left: 4px; }
			div { border: solid #5f5f5f 4px; border-radius: 10px; padding-left: 4px; margin: 2ex 10%; }
			div div { border: solid #5f5f5f; border-radius: 0; border-width: 0; margin: 1ex 1em; padding: 2px; }
			div div div { background: #cfcfcf; border: solid #5f5f5f; border-radius: 0; border-width: 0 0 0 2px; margin: 0; padding: 2px; }
			ul { font-size: 86%; padding: 0 0.5em; margin: 0 1em; }
			h1, h2 { margin: 0.5ex 2em; text-align: center; }
			h2 { text-decoration: underline; }
			h3 { margin: 0.5ex 0.25em; padding: 0; font-size: 96%; }
		</style>
	</head>
	<body>
		<h1>Shared libraries sanity report</h1>
EOT
for part in $(ls -1 -t -r ${report_parts_dir}/*); do
	cat $part >>${report_file}
done
echo -en "\t</body>\n</html>\n" >>${report_file}

if [ "$any_error_found" == true ]; then
    exit $EXIT_ON_ERROR
fi

exit 0
