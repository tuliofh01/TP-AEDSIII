---@class ImGui
local imgui = {
    Begin = function(title, open, flags) end,
    End = function() end,
    Text = function(text) end,
    Button = function(label, w, h) end,
    InputText = function(label, text) end,
    SameLine = function(offset, spacing) end,
    Spacing = function() end,
    Separator = function() end,
    SetNextWindowPos = function(x, y, cond) end,
    SetNextWindowSize = function(w, h, cond) end,
    PushStyleColor = function(idx, col) end,
    PopStyleColor = function(n) end,
    OpenPopup = function(id) end,
    CloseCurrentPopup = function() end,
    BeginPopupModal = function(name, open, flags) end,
    PushID = function(id) end,
    PopID = function() end,
}

---@return DataManager
function getDataManager() end

-- Make available globally for require if needed
return {
    imgui = imgui,
    getDataManager = getDataManager
}