/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <string.h>
#include <sys/stat.h>

static unsigned int suites_count = 0;
static unsigned int suites_max = 0;
static Suite **suites = NULL;

extern void register_suite(Suite *s);
void register_suite(Suite *s)
{
    if (suites_count == suites_max) {
        suites_max = suites_max * 2 + 7;
        Suite **suitesNew = NULL;
        suitesNew = realloc(suites, suites_max * sizeof(suites[0]));
        if (suitesNew) {
            suites = suitesNew;
        } else {
            free(suites);
            exit(1);
        }
    }
    suites[suites_count++] = s;
}

int main(int argc, char **argv)
{
    if (suites_count == 0)
        return EXIT_SUCCESS;

    char *output_path = strdup(__FILE__ ".xml");

    if (argc > 1) {
        char *tmp = NULL;
        mkdir(argv[1], 0755);
        asprintf(&tmp, "%s/%s", argv[1], output_path);
        free(output_path);
        output_path = tmp;
    }

    SRunner *runner = srunner_create(suites[0]);
    for (unsigned int i = 1; i < suites_count; i++)
        srunner_add_suite(runner, suites[i]);

    srunner_set_xml(runner, output_path);
    srunner_set_fork_status(runner, CK_NOFORK);

    srunner_run_all(runner, CK_NORMAL);

    srunner_free(runner);
    free(output_path);
    free(suites);

    return 0;
}
