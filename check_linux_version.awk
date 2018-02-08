# ***********************************************************************
# * $Id: check_linux_version.awk 82180 2018-02-02 05:50:08Z hess $
# **********************************************************************

BEGIN {
    checked = 0;
    PACKAGE_FILE = "linux_package";
    VERION_FILE   = "linux_version";
}

# Check Linux Version Script For Linux Clones.
{
    if (tolower($0) ~ /turbolinux/)
    {
        printf("turbolinux") > PACKAGE_FILE
        printf GetVersion($0) > VERION_FILE
        checked = 1;
    }
    else
    if (tolower($0) ~ /red*hat*enterprise/ || tolower($0) ~ /red *hat *enterprise/)
    {
        printf("redhat_Enterprise") > PACKAGE_FILE
        printf GetKind($0) > VERION_FILE
        printf GetVersion($0) >> VERION_FILE
        checked = 1;
    }
    else
    if (tolower($0) ~ /red*hat/ || tolower($0) ~ /red *hat/)
    {
        printf("redhat") > PACKAGE_FILE
        printf GetVersion($0) > VERION_FILE
        checked = 1;
    }
    else
    if (tolower($0) ~ /wowlinux/)
    {
        printf("wowlinux") > PACKAGE_FILE
        printf GetVersion($0) > VERION_FILE
        checked = 1;
    }
    else
    if (tolower($0) ~ /hancom/)
    {
        printf("hancom") > PACKAGE_FILE;
        printf GetVersion($0) > VERION_FILE;
        checked = 1;
            
    }
    else
    if (tolower($0) ~ /suse/ || tolower($0) ~ /s\.u\.s\.e/)
    {
        printf("suse") > PACKAGE_FILE
        printf GetVersion($0) > VERION_FILE
        checked = 1;
    }
    else
    if (tolower($0) ~ /caldera/)
    {
        printf("caldera") > PACKAGE_FILE
        printf GetVersion($0) > VERION_FILE
        checked = 1;
    }
    else
    if (tolower($0) ~ /mandrake/)
    {
        printf("mandrake") > PACKAGE_FILE
        printf GetVersion($0) > VERION_FILE
        checked = 1;
    }
    else
    if (tolower($0) ~ /booyo/)
    {
        printf("booyo") > PACKAGE_FILE
        printf GetVersion($0) > VERION_FILE
        checked = 1;
    }
    else
    if (tolower($0) ~ /asianux/)
    {
        printf("asianux") > PACKAGE_FILE
        printf GetVersion($0) > VERION_FILE
        checked = 1;
    }
    else
    if (tolower($0) ~ /ubuntu/)
    {
        printf("ubuntu") > PACKAGE_FILE
        printf GetVersion($0) > VERION_FILE
        checked = 1;
    }
    else
    if (tolower($0) ~ /ginux/)
    {
        printf("ginux") > PACKAGE_FILE
        printf GetVersion($0) > VERION_FILE
        checked = 1;
    }
    else
    if (tolower($0) ~ /fedora/)
    {
        printf("fedora") > PACKAGE_FILE
        printf GetVersion($0) > VERION_FILE
        checked = 1;
    }
    else
    if (tolower($0) ~ /linux mint/)
    {
        printf("mint") > PACKAGE_FILE
        printf GetVersion($0) > VERION_FILE
        checked = 1;
    }
    else
    if (tolower($0) ~ /centos/)
    {
        printf("centos") > PACKAGE_FILE
        printf GetVersion($0) > VERION_FILE
        checked = 1;
    }
}

END {
    if (checked == 0)
    {
        printf("Unknown") > PACKAGE_FILE;
        printf("Unknown") > VERION_FILE;
    }
}

function GetKind(str)
{
    for (i = 1; i <= NF; i++)
    {
        if ( $i ~ /[AaWwEe][Ss]/ )
        {
            return $i;
        }
    }
    return "Unknown";
}

function GetVersion(str)
{
    for (i = 1; i <= NF; i++)
    {
        if ( $i ~ /[0-9]+.?[0-9]*[a-zA-Z]*/)
        {
            return $i;
        }
    }
    return "Unknown";
}
