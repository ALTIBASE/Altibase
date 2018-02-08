# ***********************************************************************
# * $Id: check_bison_version.awk 82180 2018-02-02 05:50:08Z hess $
# **********************************************************************

BEGIN {
    check = 0;
}
{
    for (i = 1; i <= NF; i++)
    {
        if ( $i ~ /[0-9]+\.[0-9]+[a-zA-Z]*/)
        {
            seed = $i;
            printf("ok. NUMBER = %s\n", seed);
            res = split(seed, b, /\./);

            printf("Major = %s : Minor = %s : Patch = %s\n",
                   b[1], b[2], b[3]);

            check = 1;
            exit;
        }
    }
}

END {
    if (check == 0)
    {
        printf ("Can't Get  bison Version !!\n\n");
    }
    else
    {
        printf ("Ok bison Version !!\n\n");
    }
}
