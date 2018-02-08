
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;

public class conf extends JFrame implements ActionListener
{    
    static final private int BUILD_IDX = 0;
    static final private int MEMCHK_IDX = 1;
    static final private int RECOVERY_IDX = 2;
    static final private int GCC_IDX = 3;
    static final private int BIT_IDX = 4;
    static final private int COMPAT_IDX = 5;
    static final private int SMP_IDX = 6;
    static final private int MEMPAGE_IDX = 7;
    static final private int SIGTERM_IDX = 8;
    
    static final private int CONF_MAX = 9;
    
    // command;
    static final private String CMD_OK        = "ok";
    static final private String CMD_CANCEL    = "cancel";
        
    static final private String CMD_BUILD_DEBUG    = "debug";
    static final private String CMD_BUILD_PRELEASE = "prel";
    static final private String CMD_BUILD_RELEASE  = "release";
        
    static final private String CMD_MEMCHK_OFF  = "mem_off";
    static final private String CMD_MEMCHK_ON   = "mem_on";
    
    static final private String CMD_RECOVERY_OFF  = "rec_off";
    static final private String CMD_RECOVERY_ON   = "rec_on";
    
    static final private String CMD_GCC_OFF  = "gcc_off";
    static final private String CMD_GCC_ON   = "gcc_on";
    
    static final private String CMD_BIT_32   = "bit_32";
    static final private String CMD_BIT_64   = "bit_64";
    
    static final private String CMD_COMPAT_OFF   = "compat_off";
    static final private String CMD_COMPAT_ON    = "compat_on";
    
    static final private String CMD_SMP        = "smp";
    static final private String CMD_NON_SMP    = "non_smp";
    
    static final private String CMD_MEMPAGE_32   = "mem_32";
    static final private String CMD_MEMPAGE_64   = "mem_64";
    
    static final private String CMD_SIGTERM_OFF   = "sig_off";
    static final private String CMD_SIGTERM_ON    = "sig_on";
    
    String mConfStr[] = new String[CONF_MAX];
    
    public conf()
    {
        super("configure");
        
        initConfStr();
        createPane();
        
        addWindowListener(new WindowAdapter()
        {
            public void windowClosing(WindowEvent we)
            {
                System.exit(0);
            }
        });
        pack();
        setResizable(false);
        setVisible(true);
    }
    
    void initConfStr()
    {
        mConfStr[BUILD_IDX] = " --with-build_mode=debug";
        for (int i=1; i<CONF_MAX; i++)
        {
            mConfStr[i] = "";
        }
    }
    
    public void createPane()
    {
        // * BUILD
        JPanel buildPane = new JPanel();
        //buildPane.setLayout(new GridLayout(1,2));
        buildPane.setBorder(BorderFactory.createTitledBorder("BUILD"));
        
        JRadioButton debugButton = new JRadioButton("debug");
        debugButton.setActionCommand(CMD_BUILD_DEBUG);
        debugButton.setSelected(true);
        
        JRadioButton preleaseButton = new JRadioButton("pre-release");
        preleaseButton.setActionCommand(CMD_BUILD_PRELEASE);
        preleaseButton.addActionListener(this);
        
        JRadioButton releaseButton = new JRadioButton("release");
        releaseButton.setActionCommand(CMD_BUILD_RELEASE);
        releaseButton.addActionListener(this);    
        
        ButtonGroup buildGrp = new ButtonGroup();
        buildGrp.add(debugButton);
        buildGrp.add(preleaseButton);
        buildGrp.add(releaseButton);
        
        buildPane.add(debugButton);
        buildPane.add(preleaseButton);
        buildPane.add(releaseButton);
        
        // * memory check
        JPanel memchkPane = new JPanel();
        //memchkPane.setLayout(new GridLayout(1,2));
        memchkPane.setBorder(BorderFactory.createTitledBorder("memory check"));
        
        JRadioButton offButton = new JRadioButton("Off");
        offButton.setActionCommand(CMD_MEMCHK_OFF);
        offButton.addActionListener(this);
        offButton.setSelected(true);
        
        JRadioButton onButton = new JRadioButton("On");
        onButton.setActionCommand(CMD_MEMCHK_ON);
        onButton.addActionListener(this);
                
        ButtonGroup memchkGrp = new ButtonGroup();
        memchkGrp.add(offButton);
        memchkGrp.add(onButton);
        
        memchkPane.add(offButton);
        memchkPane.add(onButton);
        
        // * recovery check
        JPanel recoveryPane = new JPanel();
        //recoveryPane.setLayout(new GridLayout(1,2));
        recoveryPane.setBorder(BorderFactory.createTitledBorder("recovery check"));
        
        offButton = new JRadioButton("Off");
        offButton.setActionCommand(CMD_RECOVERY_OFF);
        offButton.addActionListener(this);
        offButton.setSelected(true);
        
        onButton = new JRadioButton("On");
        onButton.setActionCommand(CMD_RECOVERY_ON);
        onButton.addActionListener(this);
                
        ButtonGroup recoveryGrp = new ButtonGroup();
        recoveryGrp.add(offButton);
        recoveryGrp.add(onButton);
        
        recoveryPane.add(offButton);
        recoveryPane.add(onButton);
        
        // * gcc check
        JPanel gccPane = new JPanel();
        //gccPane.setLayout(new GridLayout(1,2));
        gccPane.setBorder(BorderFactory.createTitledBorder("gcc check"));
        
        offButton = new JRadioButton("Off");
        offButton.setActionCommand(CMD_GCC_OFF);
        offButton.addActionListener(this);
        offButton.setSelected(true);
        
        onButton = new JRadioButton("On");
        onButton.setActionCommand(CMD_GCC_ON);
        onButton.addActionListener(this);
                
        ButtonGroup gccGrp = new ButtonGroup();
        gccGrp.add(offButton);
        gccGrp.add(onButton);
        
        gccPane.add(offButton);
        gccPane.add(onButton);
                
        // * 32/64
        JPanel bitPane = new JPanel();
        //bitPane.setLayout(new GridLayout(1,2));
        bitPane.setBorder(BorderFactory.createTitledBorder("32/64"));
        
        JRadioButton b32Button = new JRadioButton("32bit");
        b32Button.setActionCommand(CMD_BIT_32);
        b32Button.addActionListener(this);
        
        JRadioButton b64Button = new JRadioButton("64bit");
        b64Button.setActionCommand(CMD_BIT_64);
        b64Button.addActionListener(this);
        b64Button.setSelected(true);
                
        ButtonGroup bitGrp = new ButtonGroup();
        bitGrp.add(b32Button);
        bitGrp.add(b64Button);
        
        bitPane.add(b32Button);
        bitPane.add(b64Button);
        
        // * compat4
        JPanel compatPane = new JPanel();
        //compatPane.setLayout(new GridLayout(1,2));
        compatPane.setBorder(BorderFactory.createTitledBorder("compat4"));
        
        offButton = new JRadioButton("Off");
        offButton.setActionCommand(CMD_COMPAT_OFF);
        offButton.addActionListener(this);
        offButton.setSelected(true);
        
        onButton = new JRadioButton("On");
        onButton.setActionCommand(CMD_COMPAT_ON);
        onButton.addActionListener(this);
                
        ButtonGroup compatGrp = new ButtonGroup();
        compatGrp.add(offButton);
        compatGrp.add(onButton);
        
        compatPane.add(offButton);
        compatPane.add(onButton);
        
        // * SMP
        JPanel smpPane = new JPanel();
        //smpPane.setLayout(new GridLayout(1,2));
        smpPane.setBorder(BorderFactory.createTitledBorder("SMP"));
        
        offButton = new JRadioButton("SMP");
        offButton.setActionCommand(CMD_SMP);
        offButton.addActionListener(this);
        offButton.setSelected(true);
        
        onButton = new JRadioButton("Non-SMP");
        onButton.setActionCommand(CMD_NON_SMP);
        onButton.addActionListener(this);
                
        ButtonGroup smpGrp = new ButtonGroup();
        smpGrp.add(offButton);
        smpGrp.add(onButton);
        
        smpPane.add(offButton);
        smpPane.add(onButton);
        
        // * Memory Page
        JPanel mempagePane = new JPanel();
        //mempagePane.setLayout(new GridLayout(1,2));
        mempagePane.setBorder(BorderFactory.createTitledBorder("Memory Page"));
        
        b32Button = new JRadioButton("32K");
        b32Button.setActionCommand(CMD_MEMPAGE_32);
        b32Button.addActionListener(this);
        b32Button.setSelected(true);
        
        b64Button = new JRadioButton("64K");
        b64Button.setActionCommand(CMD_MEMPAGE_64);
        b64Button.addActionListener(this);
                
        ButtonGroup mempageGrp = new ButtonGroup();
        mempageGrp.add(b32Button);
        mempageGrp.add(b64Button);
        
        mempagePane.add(b32Button);
        mempagePane.add(b64Button);
        
        // * SIGTERM Block (on linux)
        JPanel sigtermPane = new JPanel();
        //sigtermPane.setLayout(new GridLayout(1,2));
        sigtermPane.setBorder(BorderFactory.createTitledBorder("SIGTERM Block(on linux)"));
        
        offButton = new JRadioButton("Off");
        offButton.setActionCommand(CMD_SIGTERM_OFF);
        offButton.addActionListener(this);
        offButton.setSelected(true);
        
        onButton = new JRadioButton("On");
        onButton.setActionCommand(CMD_SIGTERM_ON);
        onButton.addActionListener(this);
                
        ButtonGroup sigtermGrp = new ButtonGroup();
        sigtermGrp.add(offButton);
        sigtermGrp.add(onButton);
        
        sigtermPane.add(offButton);
        sigtermPane.add(onButton);
        
        JPanel p = new JPanel(new GridLayout(1,2));
        p.setBorder(new EmptyBorder(10, 10, 10, 10));
        
        JButton okButton = new JButton("OK");
        okButton.setActionCommand(CMD_OK);
        okButton.addActionListener(this);
        p.add(okButton);

        JButton cancelButton = new JButton("Cancel");
        cancelButton.setActionCommand(CMD_CANCEL);
        cancelButton.addActionListener(this);
        p.add(cancelButton);
        
        JPanel choiceP = new JPanel();
        choiceP.setLayout(new GridLayout(5,2));
        
        choiceP.add(buildPane);
        choiceP.add(memchkPane);
        choiceP.add(recoveryPane);
        choiceP.add(gccPane);
        choiceP.add(bitPane);
        choiceP.add(compatPane);
        choiceP.add(smpPane);
        choiceP.add(mempagePane);
        choiceP.add(sigtermPane);
        choiceP.add(p);
        
        getContentPane().add(choiceP, BorderLayout.CENTER);
        getContentPane().add(p, BorderLayout.SOUTH);
    }
    
    public void actionPerformed(ActionEvent e)
    {
        String cmd = e.getActionCommand();
        
        if (CMD_OK.equals(cmd))
        {
            execConfigure();
            //System.exit(0);
        }
        else if (CMD_CANCEL.equals(cmd))
        {
            System.exit(0);
        }
        // BUILD
        else if (CMD_BUILD_DEBUG.equals(cmd))
        {
            mConfStr[BUILD_IDX] = " --with-build_mode=debug";
        }
        else if (CMD_BUILD_PRELEASE.equals(cmd))
        {
            mConfStr[BUILD_IDX] = " --with-build_mode=prerelease";
        }
        else if (CMD_BUILD_RELEASE.equals(cmd))
        {
            mConfStr[BUILD_IDX] = " --with-build_mode=release";
        }
        // memory check
        else if (CMD_MEMCHK_OFF.equals(cmd))
        {
            mConfStr[MEMCHK_IDX] = "";
        }
        else if (CMD_MEMCHK_ON.equals(cmd))
        {
            mConfStr[MEMCHK_IDX] = " --enable-memcheck";
        }
        // recovery check
        else if (CMD_RECOVERY_OFF.equals(cmd))
        {
            mConfStr[RECOVERY_IDX] = "";
        }
        else if (CMD_RECOVERY_ON.equals(cmd))
        {
            mConfStr[RECOVERY_IDX] = " --enable-reccheck";
        }
        // gcc check
        else if (CMD_GCC_OFF.equals(cmd))
        {
            mConfStr[GCC_IDX] = "";
        }
        else if (CMD_GCC_ON.equals(cmd))
        {
            mConfStr[GCC_IDX] = " --enable-gcc";
        }
        // 32/64
        else if (CMD_BIT_32.equals(cmd))
        {
            mConfStr[BIT_IDX] = " --disable-bit64";
        }
        else if (CMD_BIT_64.equals(cmd))
        {
            mConfStr[BIT_IDX] = "";
        }
        // compat4
        else if (CMD_COMPAT_OFF.equals(cmd))
        {
            mConfStr[COMPAT_IDX] = "";
        }
        else if (CMD_COMPAT_ON.equals(cmd))
        {
            mConfStr[COMPAT_IDX] = " --enable-compat4";
        }
        // SMP
        else if (CMD_SMP.equals(cmd))
        {
            mConfStr[SMP_IDX] = "";
        }
        else if (CMD_NON_SMP.equals(cmd))
        {
            mConfStr[SMP_IDX] = " --enable-nosmp";
        }
        // Memory Page
        else if (CMD_MEMPAGE_32.equals(cmd))
        {
            mConfStr[MEMPAGE_IDX] = "";
        }
        else if (CMD_MEMPAGE_64.equals(cmd))
        {
            mConfStr[MEMPAGE_IDX] = " --enable-page64";
        }
        // SIGTERM Block (on linux)
        else if (CMD_SIGTERM_OFF.equals(cmd))
        {
            mConfStr[SIGTERM_IDX] = "";
        }
        else if (CMD_SIGTERM_ON.equals(cmd))
        {
            mConfStr[SIGTERM_IDX] = " --enable-blocksigterm";
        }
    }
    
    void execConfigure()
    {
        StringBuffer execStr = new StringBuffer();
        execStr.append("./configure");
        for (int i=0; i<CONF_MAX; i++)
        {
            execStr.append(mConfStr[i]);         
        }
        try
        {
            System.out.println(execStr.toString());

            Process proc = Runtime.getRuntime().exec(execStr.toString());
            InputStream stdin = proc.getInputStream();
            InputStreamReader isr = new InputStreamReader(stdin);
            BufferedReader br = new BufferedReader(isr);
            String line = null;
            System.out.println("<OUTPUT>");
            while ( (line = br.readLine()) != null)
                System.out.println(line);
            System.out.println("</OUTPUT>");
//            int exitVal = proc.waitFor();            
//            System.out.println("Process exitValue: " + exitVal);
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }
    }
    
    public static void main(String s[]) {
        new conf();
    }
}
