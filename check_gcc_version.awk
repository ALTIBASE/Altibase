# ***********************************************************************
# * $Id: check_gcc_version.awk 82180 2018-02-02 05:50:08Z hess $
# **********************************************************************

# Check GCC Version Script
BEGIN {
    VERION_FILE   = "gcc_version";
    sCommand="gcc --version | sed -n '1s/^[^ ]* (.*) //;s/ .*$//;1p'";
    sCommand | getline sVersion;
    close(sCommand)
    print sVersion > VERION_FILE

    LD_OPTIONS_FILE = "gcc_ld_options";
    if (sVersion >= 4.6)
    {
        print "-Wl,--no-as-needed" > LD_OPTIONS_FILE
    }
    else
    {
        print "" > LD_OPTIONS_FILE
    }
}

