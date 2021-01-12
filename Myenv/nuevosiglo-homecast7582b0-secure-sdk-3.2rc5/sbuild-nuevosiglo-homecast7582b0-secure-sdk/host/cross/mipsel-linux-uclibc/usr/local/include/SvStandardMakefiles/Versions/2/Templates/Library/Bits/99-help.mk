SvHelp[help]=Lists all public make targets
SvHelp[help-all]=Lists all documented make targets
SvHelp[topics]=Lists all public documentation topics
SvHelpCommonTargets=compile install check clean help-all topics
SvHelpCommonTopics=writing-makefiles
SvHelpTopic[writing-makefiles]=Information on how to write makefiles
help:
	@echo "$(SvHilightColor)This commands lists most commonly used public make targets along with a short description$(SvNormalColor)"
	@$(foreach command,$(SvHelpCommonTargets),echo " * $(command) - $(value SvHelp[$(command)])";)
	@echo "The default goal is: $(.DEFAULT_GOAL)"

topics:
	@echo "$(SvHilightColor)This commands lists most important topics along with a short description$(SvNormalColor)"
	@$(foreach topic,$(SvHelpCommonTopics),echo " * $(topic) - $(value SvHelpTopic[$(topic)])";)

help-all:
	@echo "$(SvHilightColor)This commands lists all public make targets along with a short description$(SvNormalColor)"
	@$(foreach command,$(sort $(patsubst SvHelp[%],%,$(filter SvHelp[%],$(.VARIABLES)))),echo " $(SvHilightColor)*$(SvNoticeColor) $(command) $(SvHilightColor)- $(value SvHelp[$(command)])$(SvNormalColor)";)
