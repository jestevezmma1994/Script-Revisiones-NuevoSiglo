#!/bin/bash

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

# one argument - makefile name
function check_makefile
{
    problems_count=0

    MAKEFILE=$1

    if [ ${MAKEFILE_STRICT_WARNINGS_LENIENT} -gt 0 ] || [ ${MAKEFILE_STRICT_WARNINGS_NO} -gt 0 ]; then
        STRICT_WARNINGS=$(grep -nH '[^#]*\(kp\|Sv\)StrictWarnings[[:space:]]*:\?=' ${MAKEFILE})

        if [ ${MAKEFILE_STRICT_WARNINGS_LENIENT} -gt 0 ]; then
            output=$(echo ${STRICT_WARNINGS} | grep ':\?=[[:space:]]*lenient')
            if [ $? -eq 0 ]; then
                [ ${problems_count} -eq 0 ] && open_file ${MAKEFILE}
                open_section 'strict warnings check is disabled'
                echo ${output} | html_escape
                close_section
                problems_count=$((problems_count + 1))
            fi
        fi # MAKEFILE_STRICT_WARNINGS_LENIENT

        if [ ${MAKEFILE_STRICT_WARNINGS_NO} -gt 0 ]; then
            output=$(echo ${STRICT_WARNINGS} | grep ':\?=[[:space:]]*no')
            if [ $? -eq 0 ]; then
                [ ${problems_count} -eq 0 ] && open_file ${MAKEFILE}
                open_section 'strict warnings check is disabled'
                echo ${output} | html_escape
                close_section
                problems_count=$((problems_count + 1))
            fi
        fi # MAKEFILE_STRICT_WARNINGS_NO
    fi # MAKEFILE_STRICT_WARNINGS_LENIENT or MAKEFILE_STRICT_WARNINGS_NO

    if [ ${MAKEFILE_DESCRIPTION} -gt 0 ]; then
        DESCRIPTION=$(grep -nH '[^#]*\(SvDescription\|kpDesc\)' ${MAKEFILE})
        if [ $? -eq 0 ]; then
            output="$(echo ${DESCRIPTION} | grep '[^#]*\(SvDescription\|kpDesc\)[[:space:]]*:\?=\([[:space:]]*$\|.*todo\|.*TODO\|.*FIXME\|.*fixme\|.*__unknown__\)')"
            if [ $? -eq 0 ]; then
                [ ${problems_count} -eq 0 ] && open_file ${MAKEFILE}
                open_section 'component description is empty or contains forbidden string'
                echo ${output} | html_escape
                close_section
                problems_count=$((problems_count + 1))
            fi
        else
            [ ${problems_count} -eq 0 ] && open_file ${MAKEFILE}
            open_section 'component description not found'
            echo 'missing SvDescription or kpDesc variable' | html_escape
            close_section
            problems_count=$((problems_count + 1))
        fi
    fi # MAKEFILE_DESCRIPTION

    [ ${problems_count} -gt 0 ] && close_file
    return ${problems_count}
}

#######################################################################

function fail
{
    echo "check_makefile_sanity: $1" >&2
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
if [ $# -eq 0 ]; then
    fail 'invalid usage'
fi

MAKEFILE=$1
shift
if [ ! -z $1 ]; then
    component_name=$1
fi

# generate output file
report_parts_dir=${SRM_ROOT}/docs/makefile_sanity_report
if [ ! -d ${report_parts_dir} ] && [ -d ${SRM_ROOT}/base ]; then
    base_root_dir="${SRM_ROOT}/base/sbuild-*"
    if [ -d ${base_root_dir}/docs/makefile_sanity_report ]; then
        cp -r ${base_root_dir}/docs/makefile_sanity_report ${report_parts_dir}
    fi
fi
mkdir -p ${report_parts_dir}
echo -n >${report_parts_dir}/___empty___
report_part_file=${report_parts_dir}/${component_name}
echo -n >${report_part_file}

# do the checking
echo -en "\t\t<div><a name="${component_name}"/>\n" >>${report_part_file}
echo -en "\t\t\t<h2>Component ${component_name}</h2>\n" >>${report_part_file}

check_makefile ${MAKEFILE} >>${report_part_file}
if [ "$?" != "0" ]; then
    any_error_found=true
fi

echo -en "\t\t</div>\n" >>${report_part_file}
if [ $(wc -l <${report_part_file}) -eq 3 ]; then
    # nothing found
    rm -f ${report_part_file}
fi

# generate report file
report_file=${SRM_ROOT}/docs/makefile_sanity_report.html
cat <<EOT >${report_file}
<html>
    <head>
        <title>Makefile sanity report</title>
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
        <h1>Makefile sanity report</h1>
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
