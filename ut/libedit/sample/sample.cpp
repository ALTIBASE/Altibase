#include <idl.h>

#include <histedit.h>


char * myprompt(EditLine * /*el*/)
{
    return (char *)"MY>";
}


int main(int /* argc */, char **argv)
{
    EditLine *edo;
    History *hist;
    HistEvent ev;

    SChar      *editor;
    const char *buf;
    int         num;

    editor = idlOS::getenv("ALTIBASE_EDITOR");
    if (editor == NULL)
    {
        editor = (SChar *)"emacs";
    }

    hist = history_init();

    history(hist, &ev, H_SETSIZE,100);

    edo = el_init(*argv, stdin, stdout, stderr);
    el_set(edo, EL_PROMPT, myprompt);
    el_set(edo, EL_HIST, history, hist);
    el_set(edo, EL_SIGNAL, 1);	/* Handle signals gracefully	*/
    el_set(edo, EL_TERMINAL, NULL);
    el_set(edo, EL_EDITOR, editor);

    while(1)
    {
        buf = el_gets(edo, &num);
        if ((buf == NULL) || (num == 0))
        {
            break;
        }
        if (*buf != '\n')
        {
            history(hist, &ev, H_ENTER, buf);
        }
        (void) idlOS::fprintf(stderr, "==> got %d %s", num, buf);
    }

    return 0;
}
