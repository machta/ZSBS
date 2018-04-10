struct header {

char   rev[20];         /* Revision string                         */


char   type;            /* File type AVG=1, EEG=0                  */
char   id[20];          /* Patient ID                              */
char   oper[20];        /* Operator ID                             */
char   doctor[20];      /* Doctor ID                               */
char   referral[20];    /* Referral ID                             */
char   hospital[20];    /* Hospital ID                             */
char   patient[20];     /* Patient name                            */
short int age;          /* Patient Age                             */
char   sex;             /* Patient Sex Male='M', Female='F'        */
char   hand;            /* Handedness Mixed='M',Rt='R', lft='L'    */
char   med[20];         /* Medications                             */
char   classifi[20];    /* Classification                          */
char   state[20];       /* Patient wakefulness                     */
char   label[20];       /* Session label                           */
char   date[10];        /* Session date string                     */
char   time[12];        /* Session time strin                      */
float  mean_age;        /* Mean age (Group files only)             */            // 247
float  stdev;           /* Std dev of age (Group files only)       */
short int n;            /* Number in group file                    */
char   compfile[38];    /* Path and name of comparison file        */
float  SpectWinComp;    // Spectral window compensation factor
float  MeanAccuracy;    // Average respose accuracy
float  MeanLatency;     // Average response latency
char   sortfile[46];    /* Path and name of sort file              */            
int    NumEvents;       // Number of events in eventable                      // 353
char   compoper;        /* Operation used in comparison            */
char   avgmode;         /* Set during online averaging             */
char   review;          /* Set during review of EEG data           */
short unsigned nsweeps;      /* Number of expected sweeps               */
short unsigned compsweeps;   /* Number of actual sweeps                 */ 
short unsigned acceptcnt;    /* Number of accepted sweeps               */
short unsigned rejectcnt;    /* Number of rejected sweeps               */
short unsigned pnts;         /* Number of points per waveform           */
// short unsigned nchannels;    /* Number of active channels               */
// short unsigned avgupdate;    /* Frequency of average update             */
// char  domain;           /* Acquisition domain TIME=0, FREQ=1       */
// char  variance;         /* Variance data included flag             */
// unsigned short rate;    /* D-to-A rate                             */
// double scale;           /* scale factor for calibration            */
// char  veogcorrect;      /* VEOG corrected flag                     */
// char  heogcorrect;      /* HEOG corrected flag                     */
// char  aux1correct;      /* AUX1 corrected flag                     */
// char  aux2correct;      /* AUX2 corrected flag                     */
// float veogtrig;         /* VEOG trigger percentage                 */
// float heogtrig;         /* HEOG trigger percentage                 */
// float aux1trig;         /* AUX1 trigger percentage                 */
// float aux2trig;         /* AUX2 trigger percentage                 */
// short int heogchnl;     /* HEOG channel number                     */
// short int veogchnl;     /* VEOG channel number                     */
// short int aux1chnl;     /* AUX1 channel number                     */
// short int aux2chnl;     /* AUX2 channel number                     */
// char  veogdir;          /* VEOG trigger direction flag             */
// char  heogdir;          /* HEOG trigger direction flag             */
// char  aux1dir;          /* AUX1 trigger direction flag             */ 
// char  aux2dir;          /* AUX2 trigger direction flag             */
// short int veog_n;       /* Number of points per VEOG waveform      */
// short int heog_n;       /* Number of points per HEOG waveform      */
// short int aux1_n;       /* Number of points per AUX1 waveform      */
// short int aux2_n;       /* Number of points per AUX2 waveform      */
// short int veogmaxcnt;   /* Number of observations per point - VEOG */
// short int heogmaxcnt;   /* Number of observations per point - HEOG */
// short int aux1maxcnt;   /* Number of observations per point - AUX1 */
// short int aux2maxcnt;   /* Number of observations per point - AUX2 */
// char   veogmethod;      /* Method used to correct VEOG             */
// char   heogmethod;      /* Method used to correct HEOG             */
// char   aux1method;      /* Method used to correct AUX1             */
// char   aux2method;      /* Method used to correct AUX2             */
// float  AmpSensitivity;  /* External Amplifier gain                 */
// char   LowPass;         /* Toggle for Amp Low pass filter          */
// char   HighPass;        /* Toggle for Amp High pass filter         */
// char   Notch;           /* Toggle for Amp Notch state              */
// char   AutoClipAdd;     /* AutoAdd on clip                         */
// char   baseline;        /* Baseline correct flag                   */
// float  offstart;        /* Start point for baseline correction     */
// float  offstop;         /* Stop point for baseline correction      */
// char   reject;          /* Auto reject flag                        */
// float  rejstart;        /* Auto reject start point                 */
// float  rejstop;         /* Auto reject stop point                  */
// float  rejmin;          /* Auto reject minimum value               */
// float  rejmax;          /* Auto reject maximum value               */
// char   trigtype;        /* Trigger type                            */
// float  trigval;         /* Trigger value                           */
// char   trigchnl;        /* Trigger channel                         */
// short int trigmask;     /* Wait value for LPT port                 */
// float trigisi;          /* Interstimulus interval (INT trigger)    */
// float trigmin;          /* Min trigger out voltage (start of pulse)*/
// float trigmax;          /* Max trigger out voltage (during pulse)  */
// char  trigdir;          /* Duration of trigger out pulse           */
// char  Autoscale;        /* Autoscale on average                    */
// short int n2;           /* Number in group 2 (MANOVA)              */
// char  dir;              /* Negative display up or down             */
// float dispmin;          /* Display minimum (Yaxis)                 */
// float dispmax;          /* Display maximum (Yaxis)                 */
// float xmin;             /* X axis minimum (epoch start in sec)     */
// float xmax;             /* X axis maximum (epoch stop in sec)      */
// float AutoMin;          /* Autoscale minimum                       */
// float AutoMax;          /* Autoscale maximum                       */
// float zmin;             /* Z axis minimum - Not currently used     */
// float zmax;             /* Z axis maximum - Not currently used     */
// float lowcut;           /* Archival value - low cut on external amp*/ 
// float highcut;          /* Archival value - Hi cut on external amp */ 
// char  common;           /* Common mode rejection flag              */
// char  savemode;         /* Save mode EEG AVG or BOTH               */
// char  manmode;          /* Manual rejection of incomming data      */
// char  ref[10];          /* Label for reference electode            */
// char  Rectify;          /* Rectification on external channel       */
// float DisplayXmin;      /* Minimun for X-axis display              */
// float DisplayXmax;      /* Maximum for X-axis display              */
// char  phase;            /* flag for phase computation              */
// char  screen[16];       /* Screen overlay path name                */
// short int CalMode;      /* Calibration mode                        */
// short int CalMethod;    /* Calibration method                      */
// short int CalUpdate;    /* Calibration update rate                 */
// short int CalBaseline;  /* Baseline correction during cal          */
// short int CalSweeps;    /* Number of calibration sweeps            */           
// float CalAttenuator;    /* Attenuator value for calibration        */
// float CalPulseVolt;     /* Voltage for calibration pulse           */
// float CalPulseStart;    /* Start time for pulse                    */
// float CalPulseStop;     /* Stop time for pulse                     */  
// float CalFreq;          /* Sweep frequency                         */  
// char  taskfile[34];     /* Task file name                          */
// char  seqfile[34];      /* Sequence file path name                 */
// char  SpectMethod;      // Spectral method
// char  SpectScaling;     // Scaling employed
// char  SpectWindow;      // Window employed
// float SpectWinLength;   // Length of window %
// char  SpectOrder;       // Order of Filter for Max Entropy method
// char  NotchFilter;      // Notch Filter in or out
// char  unused[11];       // Free space


// short  FspStopMethod;   /* FSP - Stoping mode                      */
// short  FspStopMode;     /* FSP - Stoping mode                      */
// float FspFValue;        /* FSP - F value to stop terminate         */
// short int FspPoint;     /* FSP - Single point location             */
// short int FspBlockSize; /* FSP - block size for averaging          */
// unsigned short FspP1;   /* FSP - Start of window                   */
// unsigned short FspP2;   /* FSP - Stop  of window                   */
// float FspAlpha;         /* FSP - Alpha value                       */
// float FspNoise;         /* FSP - Signal to ratio value             */
// short int FspV1;        /* FSP - degrees of freedom                */   
// char  montage[40];      /* Montage file path name                  */   
// char  EventFile[40];    /* Event file path name                    */   
// float fratio;           /* Correction factor for spectral array    */
// char  minor_rev;        /* Current minor revision                  */
// short int eegupdate;    /* How often incomming eeg is refreshed    */ 
// char   compressed;      /* Data compression flag                   */
// float  xscale;          /* X position for scale box - Not used     */
// float  yscale;          /* Y position for scale box - Not used     */
// float  xsize;           /* Waveform size X direction               */
// float  ysize;           /* Waveform size Y direction               */
// char   ACmode;          /* Set SYNAP into AC mode                  */
// unsigned char   CommonChnl;      /* Channel for common waveform             */
// char   Xtics;           /* Scale tool- 'tic' flag in X direction   */ 
// char   Xrange;          /* Scale tool- range (ms,sec,Hz) flag X dir*/ 
// char   Ytics;           /* Scale tool- 'tic' flag in Y direction   */ 
// char   Yrange;          /* Scale tool- range (uV, V) flag Y dir    */ 
// float  XScaleValue;     /* Scale tool- value for X dir             */
// float  XScaleInterval;  /* Scale tool- interval between tics X dir */
// float  YScaleValue;     /* Scale tool- value for Y dir             */
// float  YScaleInterval;  /* Scale tool- interval between tics Y dir */
// float  ScaleToolX1;     /* Scale tool- upper left hand screen pos  */
// float  ScaleToolY1;     /* Scale tool- upper left hand screen pos  */
// float  ScaleToolX2;     /* Scale tool- lower right hand screen pos */
// float  ScaleToolY2;     /* Scale tool- lower right hand screen pos */
// short int port;         /* Port address for external triggering    */
// unsigned long  NumSamples;       /* Number of samples in continous file     */
// char  FilterFlag;       /* Indicates that file has been filtered   */
// float LowCutoff;        /* Low frequency cutoff                    */
// short int LowPoles;     /* Number of poles                         */
// float HighCutoff;       /* High frequency cutoff                   */ 
// short int HighPoles;    /* High cutoff number of poles             */
// char  FilterType;       /* Bandpass=0 Notch=1 Highpass=2 Lowpass=3 */
// char  FilterDomain;     /* Frequency=0 Time=1                      */
// char  SnrFlag;          /* SNR computation flag                    */
// char  CoherenceFlag;    /* Coherence has been  computed            */
// char  ContinousType;    /* Method used to capture events in *.cnt  */ 
// unsigned long  EventTablePos;    /* Position of event table                 */ 
// float ContinousSeconds; // Number of seconds to displayed per page
// long  ChannelOffset;    // Block size of one channel in SYNAMPS 
// char  AutoCorrectFlag;  // Autocorrect of DC values
// unsigned char DCThreshold; // Auto correct of DC level 

};
