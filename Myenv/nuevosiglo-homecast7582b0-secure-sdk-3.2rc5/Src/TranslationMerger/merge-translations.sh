#!/bin/bash


####################################################################################
# This script merges translation domains from many applications and puts final
# .mo files to appropriate directory in target/root.
#
# Input is a set of flags:
#  MERGE_TRANSLATIONS_DOMAIN_TAGS - it is a dictionary (string) -> (domain name)
#  MERGE_TRANSLATIONS_ORDER_<string> - where <string> is an element from TAGS dictionary
#  
# MERGE_TRANSLATIONS_DOMAIN_TAGS is used to specify which domains should be 
#   taken into account.
# MERGE_TRANSLATIONS_ORDER_<string> is used to specify the order in which .po files
#   should be merged 
#
# Flags format:
# MERGE_TRANSLATIONS_DOMAIN_TAGS=((tag=domain name),)*(tag=domain name)
# MERGE_TRANSLATIONS_ORDER_<string>=(string,)*string
#
# Example case:
# App1 installs T1 and T2 translations for domain DOM1
# App2 installs T1 translations for domain DOM2
# App3 installs T2 translations for domain DOM1
# App4 installs T1 T2 tranlsations from domains DOM1 DOM2
#
# App3 wants to add own translations and overwrite some from APP1
# App4 wants to add own translations and overwrite some from APP1, APP2 and APP3
#
# MERGE_TRANSLATIONS_DOMAIN_TAGS=tag1=DOM1,tag2=DOM2
# MERGE_TRANSLATIONS_ORDER_tag1=App1,App3,App4
# MERGE_TRANSLATIONS_ORDER_tag2=App1,App2
#
#
# Technical details:
#  tags exist only because domain can have arbitrary name and it would be impossible
#  in some cases to have MERGE_TRANSLATIONS_ORDER_DOMAIN_NAME as DOMAIN_NAME can be
#  X-Y and - is not allowed in variable names.
#
####################################################################################

TRANS_DIR="$CROSS_ROOT/usr/local/share/translations/"
#
# We replace all ',' in MERGE_TRANSLATIONS_DOMAIN_TAGS with spaces
# so we can effortlessy create an array of touples (tag,domain name)
DOMAIN_TAGS=${MERGE_TRANSLATIONS_DOMAIN_TAGS//,/ }
DOMAIN_TAGS=($DOMAIN_TAGS)

tempFile=`mktemp -d /tmp/translation_merger.tmp.XXXXXX`

function run()
{
    log=`$@ 2>&1`
    if [ $? -ne 0 ]; then
        echo "FAILED: $@"
        echo $log
        exit 1
    fi
}

function generate_test_language()
{
    echo "    Generating test language"
    TEST_LANGUAGE=$TRANS_DIR/orig/$domain/la.po
    msgen $result -o $TEST_LANGUAGE
    msgfilter --keep-header -i $TEST_LANGUAGE -o $TEST_LANGUAGE sed -e 's/\(.*\)/[\1]/'
}

for domaintag in ${DOMAIN_TAGS[@]}; do
    domaintag=${domaintag//=/ }
    domaintag=($domaintag)
    
    tag=${domaintag[0]}
    domain=${domaintag[1]}
     
    echo "Processing domain $domain"

    #
    # Inside a variable we have name of another variable whose value we
    # want to know, so we need do use ${!} operator.
    # 
    DOMAIN_ORDER_STRING="MERGE_TRANSLATIONS_ORDER_""$tag"
    DOMAIN_ORDER_STRING=${!DOMAIN_ORDER_STRING}
    DOMAIN_ORDER_STRING=${DOMAIN_ORDER_STRING//,/ }
    DOMAIN_ORDER=($DOMAIN_ORDER_STRING)

    echo "    Processing templates"
    result="$tempFile/$domain.pot"

    for potfile in $(find $TRANS_DIR/templates -name '*.pot'); do
        owner=$(basename "$potfile" .pot)
        if [ -f "$potfile" ]; then
            echo "      Merging $owner"

            if [ ! -f $result ]; then
                run cp $potfile $result
            else
                run msgcat -s --use-first $potfile $result -o $result
            fi
        fi
    done

    if [ -f $result ]; then
        run mkdir -p "$TRANS_DIR/out"
        run cp $result "$TRANS_DIR/out/$domain.pot"
        generate_test_language
    fi

    rm $result &> /dev/null

    echo "    Processing translations"
    LANGUAGUES=`find $TRANS_DIR/orig/$domain/* -type f -maxdepth 1 2>/dev/null`;
    for language in $LANGUAGUES; do
        languageName=`basename $language`
        languageName=${languageName//.po}
        echo "      Merging $languageName"
        if [ -f "$TRANS_DIR/out/$domain.pot" ]; then
            run msgmerge -s --force-po -o $result -N $language $TRANS_DIR/out/$domain.pot
        else
            cp $language $result
        fi
        run mkdir -p $DESTDIR/usr/local/share/locale/$languageName/LC_MESSAGES/ 
        run msgfmt -v -f -o "$DESTDIR/usr/local/share/locale/$languageName/LC_MESSAGES/$domain.mo" $result
        run mkdir -p "$TRANS_DIR/out/$domain/"
        run cp $result "$TRANS_DIR/out/$domain/`basename $language`"
    done
done

rm -rf $tempFile
