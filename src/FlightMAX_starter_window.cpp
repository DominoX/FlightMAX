
// All our headers combined
#include "FlightMAX.h"

// Image processing (for reading "imgui_demo.jpg"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//
// MARK: ImGui extension: formatted IDs
//

namespace ImGui {

    /// @brief Helper for creating unique IDs
    /// @details Required when creating many widgets in a loop, e.g. in a table
    IMGUI_API void PushID_formatted(const char* format, ...)    IM_FMTARGS(1);

    IMGUI_API void PushID_formatted(const char* format, ...)
    {
        // format the variable string
        va_list args;
        char sz[500];
        va_start (args, format);
        vsnprintf(sz, sizeof(sz), format, args);
        va_end (args);
        // Call the actual push function
        PushID(sz);
    }

    /// @brief Button with on-hover popup helper text
    /// @param label Text on Button
    /// @param tip Tooltip text when hovering over the button (or NULL of none)
    /// @param colFg Foreground/text color (optional, otherwise no change)
    /// @param colBg Background color (optional, otherwise no change)
    /// @param size button size, 0 for either axis means: auto size
    IMGUI_API bool ButtonTooltip(const char* label, const char* tip = nullptr, ImU32 colFg = IM_COL32(1,1,1,0), ImU32 colBg = IM_COL32(1,1,1,0), const ImVec2& size = ImVec2(0,0))
    {
        // Setup colors
        if (colFg != IM_COL32(1,1,1,0))
            ImGui::PushStyleColor(ImGuiCol_Text, colFg);
        if (colBg != IM_COL32(1,1,1,0))
            ImGui::PushStyleColor(ImGuiCol_Button, colBg);

        // do the button
        bool b = ImGui::Button(label, size);
    
        // restore previous colors
        if (colBg != IM_COL32(1,1,1,0))
            ImGui::PopStyleColor();
        if (colFg != IM_COL32(1,1,1,0))
            ImGui::PopStyleColor();

        // do the tooltip
        if (tip && ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", tip);
    
        // return if button pressed
        return b;
    }

}

//
// MARK: Global data and functions
//

// The raw TTF data of OpenFontIcons has been generated into the following file
#include "fa-solid-900.inc"

// Initial data for the example table
ImguiWidget::tableDataListTy TABLE_CONTENT = {
    {"6533","MH-65C Dolphin","AS65","United States Coast Guard",0.0f,false},
    {"N493TR","SR22T","S22T","Aircraft Guaranty Corp Trustee",0.0f,true},
    {"N77FK","G-IV","GLF4","Wilmington Trust Co Trustee",0.0f,true},
    {"N911XB","EC135T1","EC35","Air Med Services Llc",0.0f,false},
    {"OY-JRJ","Avions de Transport Regional ATR 42 310","AT43","Danish Air Transport",0.0f,false},
    {"CB-8001","C-17A Globemaster III","C17","Indian Air Force",0.0f,false},
    {"G-DVIP","AGUSTA A109E","A109","Castle Air",0.0f,true},
    {"OE-KSD","91 D Safir","SB91","Patrick Lohr",0.0f,false},
    {"D-ITOR","Citation CJ2+","C25A","Hormann Kg",0.0f,false},
    {"N544XL","Citation Excel","C56X","High Tec Industries Services Inc",0.0f,true},
    {"N368MS","R44 II","R44","Silvestri Mark J",0.0f,true},
    {"N451QX","DHC-8-402","DH8D","Horizon Air Industries Inc",0.0f,true},
    {"N1125J","1125 WESTWIND ASTRA","ASTR","Djb Air Llc",0.0f,false},
    {"N250SH","AS 350 B2","AS50","Sundance Helicopters Inc",0.0f,true},
};

// To show how global values synch between window instances we declare here 2 global variables
// Values in node "Drag Controls"
float       g_dragVal1  = 0.0f;
int         g_dragVal2  = 0;


// Trying to find a way to get a image to be displayed
const std::string IMAGE_NAME = "./Resources/plugins/FlightMAX/FlightMAX_demo.jpg";

// Font size, also roughly defines height of one line
constexpr float FONT_SIZE = 15.0f;

/// Uses "stb_image" library to load a picture into memory
/// @param fileName Path to image file
/// @param[out] imgWidth Image width in pixel
/// @param[out] imgHeight Image height in pixel
/// @return texture id
/// @exception std::runtime_error if image not found
int loadImage(const std::string& fileName, int& imgWidth, int& imgHeight) {
    int nComps;
    uint8_t *data = stbi_load(fileName.c_str(), &imgWidth, &imgHeight, &nComps, sizeof(uint32_t));

    if (!data) {
        throw std::runtime_error(std::string("Couldn't load image: ") + stbi_failure_reason());
    }

    int id;
    XPLMGenerateTextureNumbers(&id, 1);
    XPLMBindTexture2d(id, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei  (GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D   (GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    return id;
}

/// Wrapper around loadImage() capturing the possible exception
/// @param fileName Path to image file
/// @param[out] imgSize Image size in pixel
/// @return texture id for loaded image, or 0 in case of failure
int try2load_image(const std::string& fileName, ImVec2& imgSize) {
    try {
        int imgWidth=0, imgHeight=0;
        int ret = loadImage(fileName, imgWidth, imgHeight);
        imgSize.x = float(imgWidth);
        imgSize.y = float(imgHeight);
        return ret;
    } catch (const std::exception &e) {
        std::string err = std::string("FlightMAX Error: ") + e.what() + " in " + fileName + "\n";
        XPLMDebugString(err.c_str());
        return 0;
    }
}

// Helper: Turns a string upper case
inline std::string& toupper (std::string& s)
{
    std::for_each(s.begin(), s.end(), [](char& c) { c = toupper(c); });
    return s;
}

void configureImgWindow()
{
  ImgWindow::sFontAtlas = std::make_shared<ImgFontAtlas>();

  // use actual parameters to configure the font, or use one of the other methods.

  // this is a post from kuroneko on x-plane.org explaining this use.

  // Basic setup looks something like:
  // To avoid bleeding VRAM like it's going out of fashion, there is only one font atlas shared over all ImgWindows
  // and we keep the manged pointer to it in the ImgWindow class statics.

  // I use the C++11 managed/smart pointers to enforce RAII behaviours rather than encouraging use of new/delete.
  //  This means the font atlas will only get destroyed when you break all references to it.
  // (ie: via ImgWindow::sFontAtlas.reset())  You should never really need to do that though,
  // unless you're being disabled (because you'll lose your texture handles anyway and it's probably a good idea
  // to forcibly tear down the font atlas then).

  // It's probably a bug that the instance of ImgWindow doesn't actually take a copy of the shared_ptr to ensure
  // the font atlas remains valid until it's destroyed.  I was working on a lot of things when I threw that update
  // together and I was heading down that path, but I think I forgot to finish it.


  // you can use any of these fonts that are provided with X-Plane or find you own.
  // Currently you can only load one font and not sure if this might change in the future.
  // ImgWindow::sFontAtlas->AddFontFromFileTTF("./Resources/fonts/DejaVuSans.ttf", FONT_SIZE);
  // ImgWindow::sFontAtlas->AddFontFromFileTTF("./Resources/fonts/DejaVuSansMono.ttf", FONT_SIZE);
  // ImgWindow::sFontAtlas->AddFontFromFileTTF("./Resources/fonts/Inconsolata.ttf", FONT_SIZE);
  // ImgWindow::sFontAtlas->AddFontFromFileTTF("./Resources/fonts/ProFontWindows", FONT_SIZE);
  // ImgWindow::sFontAtlas->AddFontFromFileTTF("./Resources/fonts/Roboto-Bold.ttf", FONT_SIZE);
  // ImgWindow::sFontAtlas->AddFontFromFileTTF("./Resources/fonts/RobotoCondensed-Regular.ttf", FONT_SIZE);
  // ImgWindow::sFontAtlas->AddFontFromFileTTF("./Resources/fonts/Roboto-Light.ttf", FONT_SIZE);
  // ImgWindow::sFontAtlas->AddFontFromFileTTF("./Resources/fonts/Roboto-Regular.ttf", FONT_SIZE);
  // ImgWindow::sFontAtlas->AddFontFromFileTTF("./Resources/fonts/tahomabd.ttf", FONT_SIZE);

    ImgWindow::sFontAtlas->AddFontFromFileTTF("./Resources/fonts/DejaVuSansMono.ttf", FONT_SIZE);
    
    // Now we merge some icons from the OpenFontsIcons font into the above font
    // (see `imgui/docs/FONTS.txt`)
    ImFontConfig config;
    config.MergeMode = true;
    
    // We only read very selectively the individual glyphs we are actually using
    // to safe on texture space
    static ImVector<ImWchar> icon_ranges;
    ImFontGlyphRangesBuilder builder;
    // Add all icons that are actually used (they concatenate into one string)
    builder.AddText(ICON_FA_TRASH_ALT ICON_FA_SEARCH
                    ICON_FA_EXTERNAL_LINK_SQUARE_ALT
                    ICON_FA_WINDOW_MAXIMIZE ICON_FA_WINDOW_MINIMIZE
                    ICON_FA_WINDOW_RESTORE ICON_FA_WINDOW_CLOSE);
    builder.BuildRanges(&icon_ranges);

    // Merge the icon font with the text font
    ImgWindow::sFontAtlas->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data,
                                                          fa_solid_900_compressed_size,
                                                          FONT_SIZE,
                                                          &config,
                                                          icon_ranges.Data);
}

// Undo what we did in configureImgWindow()
void cleanupAfterImgWindow()
{
    // We just destroy the font atlas
    ImgWindow::sFontAtlas.reset();
}

//
// MARK: ImguiWidget (our example implementation of ImguiWindow)
//

// texture number and size of the image we want to show
// (static, because we want to load the image into a texture just once)
int      ImguiWidget::image_id = 0;
ImVec2   ImguiWidget::image_size;

// Counter for the number of windows opened
int      ImguiWidget::num_win = 0;

// Does any text contain the characters in s?
bool ImguiWidget::tableDataTy::contains (const std::string& s) const
{
    // try finding s in all our texts
    for (const std::string& t: {reg, model, typecode, owner} )
    {
        std::string l = t;
        if (toupper(l).find(s) != std::string::npos)
            return true;
    }
    
    // not found
    return false;
}

ImguiWidget::ImguiWidget(int left, int top, int right, int bot,
                         XPLMWindowDecoration decoration,
                         XPLMWindowLayer layer) :
    ImgWindow(left, top, right, bot, decoration, layer),
    myWinNum(++num_win)             // assign a unique window number
{
    // Disable reading/writing of "imgui.ini"
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    
    // We take the parameter combination "SelfDecorateResizeable" + "LayerFlightOverlay" to mean: simulate HUD
    if (decoration == xplm_WindowDecorationSelfDecoratedResizable && layer == xplm_WindowLayerFloatingWindows)
    {
        // fairly transparent, barely visible background
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_WindowBg] = ImColor(0, 0, 0, 0x10);
        
        // There's no window decoration, so to move the window we need to
        // activate a "drag area", here a small strip (roughly double text height)
        // at the top of the window, ie. the window can be moved by
        // dragging a spot near the window's top
        SetWindowDragArea(0, 5, INT_MAX, 5 + 2 * int(FONT_SIZE));
    }

    // Create a flight loop id, but don't schedule it yet
    XPLMCreateFlightLoop_t flDef = {
        sizeof(flDef),                              // structSize
        xplm_FlightLoop_Phase_BeforeFlightModel,    // phase
        cbFlightLoop,                               // callbackFunc
        (void*)this,                                // refcon
    };
    flId = XPLMCreateFlightLoop(&flDef);
    
    // Define our own window title
    //SetWindowTitle("FlightMAX v" IMGUI_VERSION " for XP11. (c) Dave Svab");
    SetWindowTitle("FlightMAX v" IMGUI_VERSION " for XP11. (c) Dave Svab");
    SetWindowResizingLimits(100, 100, 1024, 1024);
    SetVisible(true);
    
    // Initialize the list content
    listContent = {
        "1st line", "2nd line", "3rd line", "4th line", "5th line",
        "6th line", "7th line", "8th line", "9th line", "10th line"
    };
    
    // if not yet loaded: try loading an image for display
    if (!image_id)
        image_id = try2load_image(IMAGE_NAME, image_size);
    
    // copy initial table example data, init with random heading
    tableList = TABLE_CONTENT;
    for (tableDataTy& td: tableList)
        td.heading = float(std::rand() % 3600) / 10.0f;
}

ImguiWidget::~ImguiWidget()
{
    if (flId)
        XPLMDestroyFlightLoop(flId);
}

void ImguiWidget::buildInterface() {

    float win_width = ImGui::GetWindowWidth();
    float win_height = ImGui::GetWindowHeight();

    ImGui::TextUnformatted("Hello, World!");
    
    // If we are a transparent HUD-like window then we draw 3 lines that look
    // a bit like a head...so people know where to drag the window to move it
    if (HasWindowDragArea()) {
        ImGui::SameLine();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 pos_start = ImGui::GetCursorPos();
        float x_end = ImGui::GetWindowContentRegionWidth() - 75;
        for (int i = 0; i < 3; i++) {
            draw_list->AddLine(pos_start, {x_end, pos_start.y}, IM_COL32(0xa0, 0xa0, 0xa0, 255), 1.0f);
            pos_start.y += 5;
        }
    }
    
    // Button with fixed width 30 and standard height
    // to pop out the window in an OS window
    static float btnWidth = ImGui::CalcTextSize(ICON_FA_WINDOW_MAXIMIZE).x + 5;
    const bool bBtnPopOut = !IsPoppedOut();
    const bool bBtnPopIn  = IsPoppedOut() || IsInVR();
    const bool bBtnVR     = vr_is_enabled && !IsInVR();
    int numBtn = bBtnPopOut + bBtnPopIn + bBtnVR;
    if (numBtn > 0) {
        // Setup colors for window sizing buttons
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_ScrollbarGrabActive)); // dark gray
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32_BLACK_TRANS);                           // transparent
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(ImGuiCol_ScrollbarGrab)); // lighter gray

        if (bBtnVR) {
            // Same line, but right-alinged
            ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - (numBtn * btnWidth));
            if (ImGui::ButtonTooltip(ICON_FA_EXTERNAL_LINK_SQUARE_ALT, "Move into VR"))
                nextWinPosMode = xplm_WindowVR;
            --numBtn;
        }
        if (bBtnPopIn) {
            // Same line, but right-alinged
            ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - (numBtn * btnWidth));
            if (ImGui::ButtonTooltip(ICON_FA_WINDOW_RESTORE, "Move back into X-Plane"))
                nextWinPosMode = xplm_WindowPositionFree;
            --numBtn;
        }
        if (bBtnPopOut) {
            // Same line, but right-alinged
            ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - (numBtn * btnWidth));
            if (ImGui::ButtonTooltip(ICON_FA_WINDOW_MAXIMIZE, "Pop out into separate window"))
                nextWinPosMode = xplm_WindowPopOut;
            --numBtn;
        }

        // Restore colors
        ImGui::PopStyleColor(3);

        // Window mode should be set outside drawing calls to avoid crashes
        if (nextWinPosMode >= 0)
            XPLMScheduleFlightLoop(flId, -1.0, 1);
    }

    // Grouping a few lines...
    ImGui::BeginGroup();
    ImGui::Text("Window size: width = %.0f  height = %.0f", win_width, win_height);
    
    ImGui::TextUnformatted("Two Widgets");
    ImGui::SameLine();
    ImGui::TextUnformatted("One Line.");
    ImGui::EndGroup();
    
    // ...to put them side-by-side with another group of lines
    ImGui::SameLine(0.0f, 20.0f);
    ImGui::BeginGroup();
    ImGui::Text("Window #%d", myWinNum);
    ImGui::Text("X-Plane Window Id: %p", GetWindowId());
    ImGui::EndGroup();

    if (ImGui::TreeNode("Styling Widgets")) {
        const char *text = "Centered Text";
        ImVec2 text_size = ImGui::CalcTextSize(text, NULL, true);
        float center = win_width / 2 - text_size[0] / 2;
        ImGui::SetCursorPosX(center);
        ImGui::TextUnformatted(text);
        ImVec4 col = ImColor(43, 101, 236, 255);
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::TextUnformatted("Ocean Blue Text");
        ImGui::TextUnformatted("Also Ocean Blue");
        ImGui::PopStyleColor();
        ImGui::TextUnformatted("No longer Ocean Blue");
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Buttons")) {
        if (ImGui::Button("Push Me")) {
            makeRed = !makeRed;
        }

        if (makeRed) {
            ImVec4 col = ImColor(255, 0, 0, 255);
            ImGui::PushStyleColor(ImGuiCol_Text, col);
        } else {
            ImVec4 col = ImColor(0, 255, 0, 255);
            ImGui::PushStyleColor(ImGuiCol_Text, col);
        }

        ImGui::TextUnformatted("Some Text");
        ImGui::PopStyleColor();

        ImGui::Button("Click and Hold");
        if (ImGui::IsItemActive()) {
            makeRed = !makeRed;
        }

        if (ImGui::RadioButton("Choice 1", radioChoice == 1)) {
            radioChoice = 1;
        }

        if (ImGui::RadioButton("Choice 2", radioChoice == 2)) {
            radioChoice = 2;
        }

        if (ImGui::RadioButton("Choice 3", radioChoice == 3)) {
            radioChoice = 3;
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Checkboxes")) {
        ImGui::Checkbox("Make text red", &makeRed);
        if (makeRed) {
            ImVec4 col = ImColor(255, 0, 0, 255);
            ImGui::PushStyleColor(ImGuiCol_Text, col);
        } else {
            ImVec4 col = ImColor(0, 255, 0, 255);
            ImGui::PushStyleColor(ImGuiCol_Text, col);
        }

        ImGui::TextUnformatted("Some Text");
        ImGui::PopStyleColor();

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Sliders")) {
        ImGui::SliderFloat("Slider [0..1000]", &sliderVal1, 0, 1000, "Value %.2f");
        ImGui::SliderFloat("Power Slider [0..100000]", &sliderVal2, 0, 100000, "Value %.2f", 3.0);
        ImGui::SliderInt("Int Slider [0..100]", &sliderVal3, 0, 100, "Value %d");
        ImGui::SliderAngle("Angle Slider", &sliderAngle, -180, 180);

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("ComboBox")) {
        static const char * choices[] = {"Choice 1", "Choice 2", "Choice 3"};
        if (ImGui::BeginCombo("Combo Box", choices[choice1])) {
            for (int i = 0; i < 3; i++) {
                if (ImGui::Selectable(choices[i], choice1 == i))
                    choice1 = i;
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Combo Box 2", "", ImGuiComboFlags_NoPreview)) {
            if (ImGui::Selectable("Choice A", choice2 == 1)) {
                choice2 = 1;
            }
            if (ImGui::Selectable("Choice B", choice2 == 2)) {
                choice2 = 2;
            }
            if (ImGui::Selectable("Choice C", choice2 == 3)) {
                choice2 = 3;
            }
            ImGui::EndCombo();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Drag Controls / Global Variables")) {
        ImGui::DragFloat("Drag Float", &g_dragVal1, 1.0, 0, 1000, "%.2f", 1.0);
        ImGui::DragInt("Drag Int", &g_dragVal2, 1.0, 0, 1000, "%d");
        ImGui::TextUnformatted("Note: These values are global and synched between windows.");
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Plots")) {
        static float values[] = {1, 3, 2, 5, 0, 6, -3};
        ImGui::PlotLines("Value Plot", values, IM_ARRAYSIZE(values));
        ImGui::PlotLines("", values, IM_ARRAYSIZE(values) - 1, 1, "Plot", -10, FLT_MAX, ImVec2(win_width, 100));
        ImGui::PlotHistogram("Value Histogram", values, IM_ARRAYSIZE(values));
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Input")) {
        // Uses stdlib wrapper implemented in imgui/misc/cpp/imgui_stdlib.c/.h
        static char text2[1024 * 16] =
            "/*\n"
            " The Pentium F00F bug, shorthand for F0 0F C7 C8,\n"
            " the hexadecimal encoding of one offending instruction,\n"
            " more formally, the invalid operand with locked CMPXCHG8B\n"
            " instruction bug, is a design flaw in the majority of\n"
            " Intel Pentium, Pentium MMX, and Pentium OverDrive\n"
            " processors (all in the P5 microarchitecture).\n"
            "*/\n\n"
            "label:\n"
            "\tlock cmpxchg8b eax\n";
        static char text3[1024] = "";
        ImGui::InputText ("One Line Text", &userText);
        ImGui::InputTextMultiline ("Multiline Text", text2, IM_ARRAYSIZE(text2));
        ImGui::InputTextWithHint ("Text With a Hint", "hint", text3, IM_ARRAYSIZE(text3));
        ImGui::InputInt("Input int", &userI1);
        ImGui::InputInt("Input int2", &userI2, 10);

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Drawing")) {
        // local cx, cy = imgui.GetCursorScreenPos()
        ImVec2 pos = ImGui::GetCursorScreenPos();

        // -- Create a kind of canvas of size 100x100.
        // -- The previous variable remembers the top left
        // -- edge of the canvas so we can use it as relative
        // -- coordinates to draw inside the widget using
        // -- absolute coordinates.
        // imgui.Dummy(100, 100)
        ImVec2 canvas_size(100, 100);
        ImGui::Dummy(canvas_size);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        float thickness = 2.0;
        // -- Parameters: x1, y1, x2, y2, color, thickness
        // ImGui::DrawList_AddLine(cx, cy, cx + 20, cy + 20, 0xFF0000FF, 2);
        // ImGui::DrawList_AddLine(cx, cy + 20, cx + 20, cy, 0xFF0000FF, 2);
        draw_list->AddLine(ImVec2(pos.x, pos.y), ImVec2(pos.x + 20, pos.y + 20), IM_COL32(255, 0, 0, 255), thickness);
        draw_list->AddLine(ImVec2(pos.x, pos.y + 20), ImVec2(pos.x + 20, pos.y), IM_COL32(255, 0, 0, 255), thickness);

        float roundness = 0.5;
        // -- Prameters: x1, y1, x2, y2, color, roundness
        // imgui.DrawList_AddRect(cx, cy, cx + 20, cy + 20, 0xFFFF0000, 0.5);
        draw_list->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + 20, pos.y + 20), IM_COL32(0, 0, 255, 255), roundness);

        // -- Prameters: x1, y1, x2, y2, color, roundness
        // imgui.DrawList_AddRectFilled(cx + 20, cy, cx + 40, cy + 20, 0xFFFF0000, 0.5);
        draw_list->AddRectFilled(ImVec2(pos.x + 20, pos.y), ImVec2(pos.x + 40, pos.y + 20), IM_COL32(0, 0, 255, 255), roundness);

        // -- Parameters: x1, y1, x2, y2, x3, y3, color
        // imgui.DrawList_AddTriangle(cx, cy + 50, cx + 40, cy + 50, cx + 20, cy + 20, 0xFF00FF00);
        draw_list->AddTriangle(ImVec2(pos.x, pos.y + 50), ImVec2(pos.x + 40, pos.y + 50), ImVec2(pos.x + 20, pos.y + 20), IM_COL32(0, 255, 0, 255), thickness);

        // -- Parameters: x1, y1, x2, y2, x3, y3, color
        // imgui.DrawList_AddTriangleFilled(cx + 40, cy + 50, cx + 80, cy + 50, cx + 40, cy + 20, 0xFF00FF00);
        draw_list->AddTriangleFilled(ImVec2(pos.x + 40, pos.y + 50), ImVec2(pos.x + 80, pos.y + 50), ImVec2(pos.x + 40, pos.y + 20), IM_COL32(0, 255, 0, 255));

        // -- Parameters: x1, y1, r, color
        // imgui.DrawList_AddCircle(cx + 80, cy + 20, 20, 0xFF00FF00);
        draw_list->AddCircle(ImVec2(pos.x + 80, pos.y + 20), 20, IM_COL32(0, 255, 0, 255));

        // -- Parameters: x1, y1, r, color
        // imgui.DrawList_AddCircleFilled(cx + 80, cy + 20, 10, 0xFF00FF00);
        draw_list->AddCircleFilled(ImVec2(pos.x + 80, pos.y + 20), 10, IM_COL32(0, 255, 0, 255));

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Images")) {
        ImGui::Text("image_id = %d", image_id);
        // Draw a previously loaded image
        if (image_id)
            ImGui::Image((void*)(intptr_t)image_id, image_size);

        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Misc")) {
        // Create a bullet style enumeration
        ImGui::Bullet(); ImGui::TextUnformatted("Bullet");
        ImGui::Bullet(); ImGui::TextUnformatted("Style");
        ImGui::Bullet(); ImGui::TextUnformatted("Enumeration");

        // Draw a separation line
        ImGui::Separator();

        // Show a progress bar, 1 = 100%
        ImGui::ProgressBar(0.5);

        ImGui::Button("Button##1");
        ImGui::Button("Button##2");

        for (int i = 0; i < 5; i++) {
            ImGui::PushID(i);
            ImGui::Button("Button");
            ImGui::PopID();
        }

        ImGui::TreePop();
    }

    // Let#s play with lists
    if (ImGui::TreeNode("List")) {
        ImGui::ListBox ("A simple List Box", &listSelItem,
                        listContent.data(),
                        int(listContent.size()),
                        int(listContent.size())/2);  // half as high as number of list elements to show how vertical scrolling works
        ImGui::TreePop();
    }

    // Let#s play with Columns
    if (ImGui::TreeNode("Columns")) {
        ImGui::Columns(3);
        ImGui::Text("first");
        ImGui::NextColumn();
        ImGui::Text("second");
        ImGui::NextColumn();
        ImGui::Text("third");
        ImGui::NextColumn();
        ImGui::Text("first2");
        ImGui::NextColumn();
        ImGui::Text("second2");
        ImGui::NextColumn();
        ImGui::Text("third2");
        ImGui::NextColumn();
        ImGui::Text("first3");

        ImGui::Columns();

        ImGui::TreePop();

    }

    // Tables are brand-new to ImGui, as of 1.74 it is alpha status
    if (ImGui::TreeNode("Table")) {
        
        // -- Filter by text search --
        
        static char sFilter[100];
        // Note: "##SearchText" creates a unique id, but due to ## it doesn't create a label"
        if (ImGui::InputTextWithHint("##SearchText", ICON_FA_SEARCH " Search", sFilter, IM_ARRAYSIZE(sFilter),
                             ImGuiInputTextFlags_CharsUppercase |
                             ImGuiInputTextFlags_CharsNoBlank))
        {
            // determine if an item is to be shown
            for (tableDataTy& td: tableList) {
                if (sFilter[0])         // some filter defined
                    td.filtered = td.contains(sFilter);
                else                    // no filter defined -> display all
                    td.filtered = true;
            }
        }
        
        // -- The table --
        
        // Alternating rows slighly grayish
        ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, IM_COL32(0x1a, 0x1a, 0x1a, 0xff));
        // A table with pretty flexible columns, first column frozen, fully scrollable
        if (ImGui::BeginTable("Table", 7,
                              ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
                              ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable |
                              ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_SizingFixedFit |
                              ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY))
        {
            // Prepare our data: We fake some movement by turning the planes (1° per second)
            const ImGuiIO& io = ImGui::GetIO();
            for (tableDataTy& td: tableList) {
                if (td.turnsLeft) {
                    if ((td.heading -= io.DeltaTime) <    0.0f) td.heading += 360.0;
                } else {
                    if ((td.heading += io.DeltaTime) >= 360.0f) td.heading -= 360.0;
                }
            }
            
            // Set up the columns of the table
            ImGui::TableSetupColumn("Tail", ImGuiTableColumnFlags_DefaultSort, 60);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_None, 50);
            ImGui::TableSetupColumn("Model", ImGuiTableColumnFlags_None, 180);
            ImGui::TableSetupColumn("Owner", ImGuiTableColumnFlags_None, 200);
            ImGui::TableSetupColumn("Heading", ImGuiTableColumnFlags_None, 110);
            ImGui::TableSetupColumn("Left", ImGuiTableColumnFlags_None, 30);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_NoSort, 150);
            ImGui::TableSetupScrollFreeze(1,1);
            ImGui::TableHeadersRow();
            
            // Sort the data if and as needed
            ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
            if (sortSpecs && sortSpecs->SpecsDirty &&
                sortSpecs->Specs && sortSpecs->SpecsCount >= 1 &&
                tableList.size() > 1)
            {
                // We sort only by one column, no multi-column sort yet
                const ImGuiTableColumnSortSpecs& colSpec = *(sortSpecs->Specs);
                // We directly sort the tableList
                std::sort(tableList.begin(), tableList.end(),
                          [colSpec](const tableDataTy& a, const tableDataTy& b)
                {
                    int cmp =       // less than 0 if a < b
                    colSpec.ColumnIndex == 0 ? a.reg.compare(b.reg)             :
                    colSpec.ColumnIndex == 1 ? a.typecode.compare(b.typecode)   :
                    colSpec.ColumnIndex == 2 ? a.model.compare(b.model)         :
                    colSpec.ColumnIndex == 3 ? a.owner.compare(b.owner)         :
                    colSpec.ColumnIndex == 4 ? int(a.heading - b.heading)       :
                    colSpec.ColumnIndex == 5 ? a.turnsLeft - b.turnsLeft        : 0;
                    
                    return colSpec.SortDirection == ImGuiSortDirection_Ascending ?
                    cmp < 0 : cmp > 0;
                });
                sortSpecs->SpecsDirty = false;
            }

            // Here we remember which row to delete if any
            tableDataListTy::iterator delIter = tableList.end();

            // Add rows to the table
            for (auto iter = tableList.begin(); iter != tableList.end(); ++iter)
            {
                tableDataTy& td = *iter;
                
                // Skip rows which are not currently filtered
                if (!td.filtered) continue;
                
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(td.reg.c_str());
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(td.typecode.c_str());
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(td.model.c_str());
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(td.owner.c_str());
                ImGui::TableNextColumn();
                // Heading: left = red / right = green
                ImGui::TextColored(td.turnsLeft ? ImColor(255, 0, 0) : ImColor(0, 255, 0),
                                   "%03.0f", td.heading);
                // Checkbox
                ImGui::TableNextColumn();
                ImGui::PushID_formatted("Left_%p", (void*)&td); // action widget require a unique id per table row (otherwise only the first line's widget work)
                ImGui::Checkbox("", &td.turnsLeft);
                ImGui::PopID();
                
                // Actions: A few buttons
                ImGui::TableNextColumn();

                ImGui::PushID_formatted("N_%p", (void*)&td);   // North
                if (ImGui::ArrowButton("", ImGuiDir_Up))
                    td.heading = 0.0f;
                ImGui::PopID();

                ImGui::SameLine();
                ImGui::PushID_formatted("E_%p", (void*)&td);   // East
                if (ImGui::ArrowButton("", ImGuiDir_Right))
                    td.heading = 90.0f;
                ImGui::PopID();

                ImGui::SameLine();
                ImGui::PushID_formatted("S_%p", (void*)&td);   // South
                if (ImGui::ArrowButton("", ImGuiDir_Down))
                    td.heading = 180.0f;
                ImGui::PopID();

                ImGui::SameLine();
                ImGui::PushID_formatted("W_%p", (void*)&td);   // West
                if (ImGui::ArrowButton("", ImGuiDir_Left))
                    td.heading = 270.0f;
                ImGui::PopID();

                ImGui::SameLine();
                ImGui::PushID_formatted("Del_%p", (void*)&td);
                if (ImGui::ButtonTooltip(ICON_FA_TRASH_ALT, "Delete row"))
                    // remember the row to delete, but don't delete right now
                    delIter = iter;
                ImGui::PopID();
            }
            
            // Now only delete a row if requested to do so
            if (delIter != tableList.end())
                tableList.erase(delIter);

            // -- Add a row to enter new data
            static char sTail[10] = "", sType[5] = "", sModel[100] = "", sOwner[100] = "";
            static int iHead = 0;
            static bool bLeft = false;
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::InputTextWithHint("##New_Tail", "Tail", sTail, IM_ARRAYSIZE(sTail));

            ImGui::TableNextColumn();
            ImGui::InputTextWithHint("##New_Type", "Type", sType, IM_ARRAYSIZE(sType));

            ImGui::TableNextColumn();
            ImGui::InputTextWithHint("##New_Model", "Model", sModel, IM_ARRAYSIZE(sModel));

            ImGui::TableNextColumn();
            ImGui::InputTextWithHint("##New_Owner", "Owner", sOwner, IM_ARRAYSIZE(sOwner));

            ImGui::TableNextColumn();
            ImGui::InputInt("##New_Heading", &iHead);

            ImGui::TableNextColumn();
            ImGui::Checkbox("##New_TurnLeft", &bLeft);

            ImGui::TableNextColumn();
            // something entered for all texts?
            if (sTail[0] && sType[0] && sModel[0] && sOwner[0]) {
                ImGui::PushID("New_Add");
                if (ImGui::SmallButton("Add")) {
                    tableList.emplace_back(tableDataTy{
                        sTail, sModel, sType, sOwner, float(iHead), bLeft
                    });
                    // init our static text for a new entry
                    sTail[0] = '\0';
                    sType[0] = '\0';
                    sModel[0] = '\0';
                    sOwner[0] = '\0';
                    iHead = 0;
                    bLeft = false;
                }
                ImGui::PopID();
            } else {
                // There is nothing like a disabled button...
                ImGui::TextDisabled("[Add]");
            }
            
            // End of table
            ImGui::EndTable();

            
        }
        ImGui::PopStyleColor();
        
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Fonts")) {

        ImGui::Text("Default DejaVuSansMono.ttf %.1f font \n", FONT_SIZE);

        ImGui::TextUnformatted("");
        // Green color
        ImVec4 col = ImColor(0, 255, 0, 255);
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::TextUnformatted("Some Green Text with DejaVuSansMono 13 font");
        ImGui::PopStyleColor();

        ImGui::TextUnformatted("");
        // Red color
        col = ImColor(255, 0, 0, 255);
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::SetWindowFontScale(1.5);
        ImGui::TextUnformatted("Some Red Text with DejaVuSansMono 13 font with font scale of 1.5");
        ImGui::SetWindowFontScale(1.0);
        ImGui::PopStyleColor();

        ImGui::TextUnformatted("");
        // Blue color
        col = ImColor(0, 0, 255, 255);
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::SetWindowFontScale(1.25);
        ImGui::TextUnformatted("Some Blue Text with DejaVuSansMono 13 font with font scale of 1.25");
        ImGui::SetWindowFontScale(1.0);
        ImGui::PopStyleColor();

        ImGui::TextUnformatted("");
        ImGui::Text("Using ShowStyleEditor() to see if new fonts have loaded\n");
        ImGui::ShowStyleEditor();
        ImGui::TreePop();

    }
}

// Outside all rendering we can change things like window mode
float ImguiWidget::cbFlightLoop(float, float, int, void* inRefcon)
{
    // refcon is pointer to ImguiWidget
    ImguiWidget& wnd = *reinterpret_cast<ImguiWidget*>(inRefcon);

    // Has user requested a change in window mode?
    if (wnd.nextWinPosMode >= 0) {
        wnd.SetWindowPositioningMode(wnd.nextWinPosMode);
        // If we pop in, then we need to explicitely set a position for the window to appear
        if (wnd.nextWinPosMode == xplm_WindowPositionFree) {
            int left, top, right, bottom;
            wnd.GetCurrentWindowGeometry(left, top, right, bottom);
            // Normalize to our starting position (WIN_PAD|WIN_PAD), but keep size unchanged
            const int width  = right-left;
            const int height = top-bottom;
            CalcWinCoords(left, top, right, bottom);
            right  = left + width;
            bottom = top - height;
            wnd.SetWindowGeometry(left, top, right, bottom);
        }
        wnd.nextWinPosMode = -1;
    }
    
    // don't call me again
    return 0.0f;
}
