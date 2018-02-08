$(ID_DIR): $(PD_DIR) $(CORE_DIR)
makemsg: $(PD_DIR) $(CORE_DIR)
$(CORE_DIR): alemon

$(CM_DIR)/msg  \
$(SM_DIR)/msg  \
$(MT_DIR)/msg  \
$(RP_DIR)/msg  \
$(DR_DIR)/msg  \
$(QP_DIR)/msg  \
$(SD_DIR)/msg  \
$(ST_DIR)/msg  \
$(MM_DIR)/msg  \
$(UL_DIR)/msg  \
$(UT_DIR)/msg  \
$(DK_DIR)/msg: $(ID_DIR)/msg

$(CM_DIR)  \
$(SM_DIR)  \
$(MT_DIR)  \
$(RP_DIR)  \
$(DR_DIR)  \
$(QP_DIR)  \
$(SD_DIR)  \
$(ST_DIR)  \
$(MM_DIR)  \
$(UL_DIR)  \
$(UT_DIR)  \
$(DK_DIR)  \
$(TOOL_DIR): $(ID_DIR)

$(UT_DIR): $(UL_DIR) $(MT_DIR) $(CM_DIR)

$(TOOL_DIR): $(CM_DIR)

$(MM_DIR): $(CM_DIR)  \
	$(SM_DIR)  \
	$(MT_DIR)  \
	$(RP_DIR)  \
	$(DR_DIR)  \
	$(QP_DIR)  \
	$(SD_DIR)  \
	$(ST_DIR)  \
	$(DK_DIR)

$(ID_DIR)/util \
$(QP_DIR)/util \
$(SM_DIR)/util: $(MM_DIR) \
	$(CM_DIR) \
	$(SM_DIR) \
	$(MT_DIR) \
	$(RP_DIR) \
	$(DR_DIR) \
	$(QP_DIR) \
	$(ST_DIR) \
	$(DK_DIR)
