#include <Pilot.h>

#define CREATOR_ID 'BLKT'
#define PREF_ID 1
#define PREF_VER 1

typedef struct {
    UInt16 version;
    UInt8 stage;
    UInt8 hwIndex;
} GameState;

static GameState g;
static Char inputBuf[8];
static UInt8 inputLen = 0;
static Boolean adminMode = false;
static UInt8 adminTaps = 0;

static void SaveState(void) {
    g.version = PREF_VER;
    PrefSetAppPreferences(CREATOR_ID, PREF_ID, PREF_VER, &g, sizeof(g), true);
}

static void LoadState(void) {
    UInt16 size = sizeof(g);
    Int16 v = PrefGetAppPreferences(CREATOR_ID, PREF_ID, &g, &size, true);
    if (v == noPreferenceFound || g.version != PREF_VER) {
        MemSet(&g, sizeof(g), 0);
        g.version = PREF_VER;
        SaveState();
    }
}

static void ClearScreen(void) {
    RectangleType r;
    RctSetRectangle(&r, 0, 0, 160, 160);
    WinEraseRectangle(&r, 0);
}

static void Center(const Char *s, Coord y) {
    Coord w = FntCharsWidth(s, StrLen(s));
    WinDrawChars(s, StrLen(s), (160 - w)/2, y);
}

static void Button(Coord x, Coord y, Coord w, Coord h, const Char *s) {
    RectangleType r;
    RctSetRectangle(&r, x, y, w, h);
    WinDrawRectangle(&r, 2);
    WinDrawChars(s, StrLen(s), x + (w - FntCharsWidth(s, StrLen(s)))/2, y + 5);
}

static void Title(void) {
    FntSetFont(boldFont);
    Center("PROJECT BLACKLIGHT", 8);
    FntSetFont(stdFont);
    Center("SECURE ACCESS TERMINAL", 22);
    WinDrawLine(8, 34, 151, 34);
}

static void DrawKeypad(void) {
    UInt8 n = 1;
    Coord row, col;
    Char s[2]; s[1] = 0;
    for (row=0; row<3; row++) {
        for (col=0; col<3; col++) {
            s[0] = '0' + n++;
            Button(28 + col*36, 67 + row*23, 30, 19, s);
        }
    }
    Button(64, 136, 30, 19, "0");
}

static void DrawInput(void) {
    RectangleType r;
    RctSetRectangle(&r, 49, 46, 62, 16);
    WinDrawRectangle(&r, 0);
    RctSetRectangle(&r, 50, 47, 60, 14);
    WinEraseRectangle(&r, 0);
    Center(inputBuf, 49);
}

static void ShowMessage(const Char *a, const Char *b, const Char *c) {
    ClearScreen(); Title();
    if (a) Center(a, 55);
    if (b) Center(b, 70);
    if (c) Center(c, 85);
    Button(52, 120, 56, 22, "CONTINUE");
}

static void DrawStage(void) {
    ClearScreen(); Title();
    if (adminMode) {
        Center("GAME MASTER", 43);
        Button(28, 62, 104, 20, "RESET GAME");
        Button(28, 88, 104, 20, "NEXT STAGE");
        Button(28, 114, 104, 20, "EXIT ADMIN");
        return;
    }
    switch (g.stage) {
        case 0:
            Center("SYSTEM LOCKED", 50);
            Center("Recover the card index.", 68);
            Button(48, 105, 64, 24, "BEGIN");
            break;
        case 1:
            Center("CARD INDEX", 39);
            Center("Enter four digits:", 52);
            DrawInput(); DrawKeypad();
            break;
        case 2:
            Center("LEVEL 1 GRANTED", 46);
            Center("A picture may show", 64);
            Center("you one thing.", 77);
            Center("TURN IT OVER.", 96);
            Button(52, 125, 56, 22, "NEXT");
            break;
        case 3:
            Center("UV AUTHENTICATION", 39);
            Center("Enter access code:", 52);
            DrawInput(); DrawKeypad();
            break;
        case 4:
            Center("LEVEL 2 GRANTED", 45);
            Center("What appears fantasy", 64);
            Center("may contain something", 77);
            Center("very real.", 90);
            Center("FIND THE WIZARD.", 106);
            Button(52, 130, 56, 20, "NEXT");
            break;
        case 5:
            Center("CIPHER CHECK", 42);
            Center("Which word did you", 58);
            Center("recover?", 70);
            Button(19, 91, 58, 22, "MAGIC");
            Button(83, 91, 58, 22, "LIGHT");
            Button(51, 121, 58, 22, "SHADOW");
            break;
        case 6:
            Center("MANUAL OVERRIDE", 45);
            Center("SOFTWARE INPUT OFF", 62);
            Center("Use the four hard keys.", 80);
            Center("Find the override", 96);
            Center("sequence elsewhere.", 109);
            Center("Waiting...", 128);
            break;
        case 7:
            Center("OVERRIDE ACCEPTED", 45);
            Center("CRYPTEX ARCHIVE", 64);
            Center("Where light exists,", 79);
            Center("darkness follows.", 92);
            FntSetFont(boldFont); Center("SHADOW", 113); FntSetFont(stdFont);
            Button(52, 136, 56, 19, "NEXT");
            break;
        case 8:
            Center("TRANSMISSION VERIFY", 39);
            Center("Enter video code:", 52);
            DrawInput(); DrawKeypad();
            break;
        case 9:
            Center("FINAL CLEARANCE", 45);
            Center("The last key hides", 67);
            Center("beneath something", 82);
            Center("MADE OF METAL.", 99);
            Button(51, 125, 58, 22, "ESCAPED");
            break;
        default:
            Center("ACCESS GRANTED", 54);
            FntSetFont(boldFont); Center("ESCAPED", 78); FntSetFont(stdFont);
            Center("PROJECT BLACKLIGHT", 103);
            break;
    }
}

static Boolean IsInside(Coord x, Coord y, Coord bx, Coord by, Coord bw, Coord bh) {
    return (x >= bx && x < bx+bw && y >= by && y < by+bh);
}

static void ResetInput(void) { inputLen = 0; inputBuf[0] = 0; }

static void Advance(void) { g.stage++; ResetInput(); SaveState(); DrawStage(); }

static void CheckCode(const Char *code) {
    if (StrCompare(inputBuf, code) == 0) {
        SndPlaySystemSound(sndInfo);
        Advance();
    } else {
        SndPlaySystemSound(sndError);
        ResetInput(); DrawStage();
    }
}

static Boolean HandlePen(Coord x, Coord y) {
    UInt8 row, col, digit=0;
    if (!adminMode && IsInside(x,y,0,0,24,24)) {
        adminTaps++;
        if (adminTaps >= 5) { adminTaps=0; adminMode=true; DrawStage(); }
        return true;
    } else if (!IsInside(x,y,0,0,24,24)) {
        adminTaps=0;
    }
    if (adminMode) {
        if (IsInside(x,y,28,62,104,20)) { g.stage=0; g.hwIndex=0; SaveState(); adminMode=false; ResetInput(); DrawStage(); return true; }
        if (IsInside(x,y,28,88,104,20)) { if (g.stage<10) g.stage++; SaveState(); DrawStage(); return true; }
        if (IsInside(x,y,28,114,104,20)) { adminMode=false; DrawStage(); return true; }
        return true;
    }
    if (g.stage==0 && IsInside(x,y,48,105,64,24)) { Advance(); return true; }
    if (g.stage==2 && IsInside(x,y,52,125,56,22)) { Advance(); return true; }
    if (g.stage==4 && IsInside(x,y,52,130,56,20)) { Advance(); return true; }
    if (g.stage==7 && IsInside(x,y,52,136,56,19)) { Advance(); return true; }
    if (g.stage==5) {
        if (IsInside(x,y,19,91,58,22)) { Advance(); return true; }
        if (IsInside(x,y,83,91,58,22) || IsInside(x,y,51,121,58,22)) { SndPlaySystemSound(sndError); return true; }
    }
    if (g.stage==9 && IsInside(x,y,51,125,58,22)) { Advance(); return true; }
    if (g.stage==1 || g.stage==3 || g.stage==8) {
        for (row=0; row<3; row++) for (col=0; col<3; col++)
            if (IsInside(x,y,28+col*36,67+row*23,30,19)) digit = 1 + row*3 + col;
        if (IsInside(x,y,64,136,30,19)) digit = 10;
        if (digit && inputLen<4) {
            inputBuf[inputLen++] = (digit==10)?'0':('0'+digit);
            inputBuf[inputLen]=0; DrawStage();
            if (inputLen==4) {
                if (g.stage==1) CheckCode("3816");
                else if (g.stage==3) CheckCode("8152");
                else CheckCode("7294");
            }
        }
        return true;
    }
    return false;
}

static Boolean HandleHardKey(WChar chr) {
    static const WChar seq[4] = { vchrHard3, vchrHard1, vchrHard4, vchrHard2 };
    if (g.stage != 6) return false;
    if (chr == seq[g.hwIndex]) {
        g.hwIndex++;
        if (g.hwIndex >= 4) { g.hwIndex=0; Advance(); }
        else { SndPlaySystemSound(sndClick); }
    } else if (chr==vchrHard1 || chr==vchrHard2 || chr==vchrHard3 || chr==vchrHard4) {
        g.hwIndex=0; SndPlaySystemSound(sndError); DrawStage();
    }
    return true;
}

static void EventLoop(void) {
    EventType e;
    for (;;) {
        EvtGetEvent(&e, evtWaitForever);
        if (e.eType == appStopEvent) break;
        if (SysHandleEvent(&e)) continue;
        switch (e.eType) {
            case penDownEvent:
                HandlePen(e.screenX, e.screenY);
                break;
            case keyDownEvent:
                HandleHardKey(e.data.keyDown.chr);
                break;
            default:
                break;
        }
    }
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
    if (cmd != sysAppLaunchCmdNormalLaunch) return 0;
    LoadState(); ResetInput();
    ClearScreen(); DrawStage();
    EventLoop();
    SaveState();
    return 0;
}
