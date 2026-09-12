local PREFIX = "BossArchive"
local PAGE_SIZE = 10

local currentPage = 0
local totalPages = 0
local totalCount = 0
local pendingItems = {}

local currentSearch = ""
local currentRarity = "ALL"

local selectedItemEntry = nil
local selectedRow = nil

local function Split(text, delimiter)
    local result = {}
    local startPos = 1

    while true do
        local foundPos = string.find(
            text,
            delimiter,
            startPos,
            true
        )

        if not foundPos then
            table.insert(
                result,
                string.sub(text, startPos)
            )
            break
        end

        table.insert(
            result,
            string.sub(
                text,
                startPos,
                foundPos - 1
            )
        )

        startPos = foundPos + string.len(delimiter)
    end

    return result
end

local function SanitizeSearch(text)
    text = text or ""

    -- "|" is our protocol delimiter.
    text = string.gsub(text, "|", "")

    return text
end

local function SendListRequest(page)
    SendAddonMessage(
        PREFIX,
        "LIST|" ..
            page ..
            "|" ..
            currentRarity ..
            "|" ..
            currentSearch,
        "WHISPER",
        UnitName("player")
    )
end

local function SendCopyRequest(itemEntry)
    SendAddonMessage(
        PREFIX,
        "COPY|" .. itemEntry,
        "WHISPER",
        UnitName("player")
    )
end

local frame = CreateFrame(
    "Frame",
    "BossArchiveFrame",
    UIParent
)

frame:SetWidth(440)
frame:SetHeight(520)
frame:SetPoint("CENTER")
frame:SetFrameStrata("DIALOG")
frame:SetMovable(true)
frame:EnableMouse(true)
frame:RegisterForDrag("LeftButton")
frame:SetClampedToScreen(true)

frame:SetBackdrop({
    bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background",
    edgeFile = "Interface\\DialogFrame\\UI-DialogBox-Border",
    tile = true,
    tileSize = 32,
    edgeSize = 32,
    insets = {
        left = 11,
        right = 12,
        top = 12,
        bottom = 11
    }
})

frame:SetScript(
    "OnDragStart",
    function(self)
        self:StartMoving()
    end
)

frame:SetScript(
    "OnDragStop",
    function(self)
        self:StopMovingOrSizing()
    end
)

frame:Hide()

local title = frame:CreateFontString(
    nil,
    "OVERLAY",
    "GameFontNormalLarge"
)

title:SetPoint(
    "TOP",
    frame,
    "TOP",
    0,
    -18
)

title:SetText("Boss Archive")

local closeButton = CreateFrame(
    "Button",
    nil,
    frame,
    "UIPanelCloseButton"
)

closeButton:SetPoint(
    "TOPRIGHT",
    frame,
    "TOPRIGHT",
    -5,
    -5
)

-- ------------------------------------------------------------
-- Search
-- ------------------------------------------------------------

local searchBox = CreateFrame(
    "EditBox",
    nil,
    frame,
    "InputBoxTemplate"
)

searchBox:SetWidth(225)
searchBox:SetHeight(24)
searchBox:SetPoint(
    "TOPLEFT",
    frame,
    "TOPLEFT",
    32,
    -52
)

searchBox:SetAutoFocus(false)
searchBox:SetMaxLetters(60)

local searchHint = frame:CreateFontString(
    nil,
    "OVERLAY",
    "GameFontDisableSmall"
)

searchHint:SetPoint(
    "LEFT",
    searchBox,
    "LEFT",
    5,
    0
)

searchHint:SetText("Search item name...")

local function UpdateSearchHint()
    if searchBox:GetText() == "" and not searchBox:HasFocus() then
        searchHint:Show()
    else
        searchHint:Hide()
    end
end

searchBox:SetScript(
    "OnEditFocusGained",
    function()
        searchHint:Hide()
    end
)

searchBox:SetScript(
    "OnEditFocusLost",
    function()
        UpdateSearchHint()
    end
)

searchBox:SetScript(
    "OnTextChanged",
    function()
        UpdateSearchHint()
    end
)

local function ApplySearch()
    currentSearch =
        SanitizeSearch(
            searchBox:GetText()
        )

    if searchBox:GetText() ~= currentSearch then
        searchBox:SetText(currentSearch)
    end

    SendListRequest(0)
end

searchBox:SetScript(
    "OnEnterPressed",
    function(self)
        self:ClearFocus()
        ApplySearch()
    end
)

searchBox:SetScript(
    "OnEscapePressed",
    function(self)
        self:ClearFocus()
    end
)

local searchButton = CreateFrame(
    "Button",
    nil,
    frame,
    "UIPanelButtonTemplate"
)

searchButton:SetWidth(90)
searchButton:SetHeight(24)
searchButton:SetPoint(
    "LEFT",
    searchBox,
    "RIGHT",
    10,
    0
)

searchButton:SetText("Search")

searchButton:SetScript(
    "OnClick",
    function()
        searchBox:ClearFocus()
        ApplySearch()
    end
)

-- ------------------------------------------------------------
-- Rarity dropdown
-- ------------------------------------------------------------

local rarityLabel = frame:CreateFontString(
    nil,
    "OVERLAY",
    "GameFontNormalSmall"
)

rarityLabel:SetPoint(
    "TOPLEFT",
    frame,
    "TOPLEFT",
    30,
    -86
)

rarityLabel:SetText("Rarity:")

local rarityDropdown = CreateFrame(
    "Frame",
    "BossArchiveRarityDropdown",
    frame,
    "UIDropDownMenuTemplate"
)

rarityDropdown:SetPoint(
    "LEFT",
    rarityLabel,
    "RIGHT",
    -8,
    -1
)

UIDropDownMenu_SetWidth(
    rarityDropdown,
    130
)

local rarityOptions = {
    { value = "ALL",       text = "All" },
    { value = "UNCOMMON",  text = "Uncommon" },
    { value = "RARE",      text = "Rare" },
    { value = "EPIC",      text = "Epic" },
    { value = "LEGENDARY", text = "Legendary" },
    { value = "ARTIFACT",  text = "Artifact" },
    { value = "HEIRLOOM",  text = "Heirloom" }
}

UIDropDownMenu_Initialize(
    rarityDropdown,
    function()
        for _, option in ipairs(rarityOptions) do
            local value = option.value
            local text = option.text

            local info =
                UIDropDownMenu_CreateInfo()

            info.text = text
            info.value = value

            info.checked =
                currentRarity == value

            info.func =
                function()
                    currentRarity = value

                    UIDropDownMenu_SetText(
                        rarityDropdown,
                        text
                    )

                    CloseDropDownMenus()

                    SendListRequest(0)
                end

            UIDropDownMenu_AddButton(info)
        end
    end
)

UIDropDownMenu_SetText(
    rarityDropdown,
    "All"
)

-- ------------------------------------------------------------
-- Archive status
-- ------------------------------------------------------------

local countLabel = frame:CreateFontString(
    nil,
    "OVERLAY",
    "GameFontHighlightSmall"
)

countLabel:SetPoint(
    "TOPLEFT",
    frame,
    "TOPLEFT",
    30,
    -120
)

countLabel:SetText("0 items")

local statusLabel = frame:CreateFontString(
    nil,
    "OVERLAY",
    "GameFontHighlightSmall"
)

statusLabel:SetPoint(
    "TOPRIGHT",
    frame,
    "TOPRIGHT",
    -30,
    -120
)

statusLabel:SetJustifyH("RIGHT")
statusLabel:SetText("")

-- ------------------------------------------------------------
-- Item rows
-- ------------------------------------------------------------

local rows = {}
local UpdateGetButton

local function ClearSelection()
    selectedItemEntry = nil

    if selectedRow then
        selectedRow.selection:Hide()
        selectedRow = nil
    end
end

for i = 1, PAGE_SIZE do
    local row = CreateFrame(
        "Button",
        nil,
        frame
    )

    row:SetWidth(375)
    row:SetHeight(24)

    if i == 1 then
        row:SetPoint(
            "TOPLEFT",
            frame,
            "TOPLEFT",
            30,
            -145
        )
    else
        row:SetPoint(
            "TOPLEFT",
            rows[i - 1],
            "BOTTOMLEFT",
            0,
            -3
        )
    end

    local selection =
        row:CreateTexture(
            nil,
            "BACKGROUND"
        )

    selection:SetAllPoints(row)

    selection:SetTexture(
        "Interface\\QuestFrame\\UI-QuestTitleHighlight"
    )

    selection:SetBlendMode("ADD")
    selection:SetAlpha(0.65)
    selection:Hide()

    local highlight =
        row:CreateTexture(
            nil,
            "HIGHLIGHT"
        )

    highlight:SetAllPoints(row)

    highlight:SetTexture(
        "Interface\\QuestFrame\\UI-QuestTitleHighlight"
    )

    highlight:SetBlendMode("ADD")

    local text =
        row:CreateFontString(
            nil,
            "OVERLAY",
            "GameFontHighlight"
        )

    text:SetPoint(
        "LEFT",
        row,
        "LEFT",
        5,
        0
    )

    text:SetJustifyH("LEFT")

    row.text = text
    row.selection = selection
    row.itemEntry = nil

    row:SetScript(
        "OnClick",
        function(self)
            if not self.itemEntry then
                return
            end

            if selectedRow and selectedRow ~= self then
                selectedRow.selection:Hide()
            end

            selectedRow = self
            selectedItemEntry = self.itemEntry

            self.selection:Show()

            statusLabel:SetText("")

            UpdateGetButton()
        end
    )

    row:SetScript(
        "OnEnter",
        function(self)
            if not self.itemEntry then
                return
            end

            GameTooltip:SetOwner(
                self,
                "ANCHOR_RIGHT"
            )

            GameTooltip:SetHyperlink(
                "item:" .. self.itemEntry
            )

            GameTooltip:Show()
        end
    )

    row:SetScript(
        "OnLeave",
        function()
            GameTooltip:Hide()
        end
    )

    rows[i] = row
end

-- ------------------------------------------------------------
-- Get button
-- ------------------------------------------------------------

local getButton = CreateFrame(
    "Button",
    nil,
    frame,
    "UIPanelButtonTemplate"
)

getButton:SetWidth(160)
getButton:SetHeight(26)

getButton:SetPoint(
    "BOTTOM",
    frame,
    "BOTTOM",
    0,
    72
)

getButton:SetText("Get from Archive")
getButton:Disable()

getButton:SetScript(
    "OnClick",
    function()
        if not selectedItemEntry then
            return
        end

        statusLabel:SetTextColor(
            1,
            1,
            1
        )

        statusLabel:SetText(
            "Requesting item..."
        )

        SendCopyRequest(
            selectedItemEntry
        )
    end
)

-- ------------------------------------------------------------
-- Pagination
-- ------------------------------------------------------------

local previousButton = CreateFrame(
    "Button",
    nil,
    frame,
    "UIPanelButtonTemplate"
)

previousButton:SetWidth(100)
previousButton:SetHeight(24)

previousButton:SetPoint(
    "BOTTOMLEFT",
    frame,
    "BOTTOMLEFT",
    28,
    28
)

previousButton:SetText("Previous")

local nextButton = CreateFrame(
    "Button",
    nil,
    frame,
    "UIPanelButtonTemplate"
)

nextButton:SetWidth(100)
nextButton:SetHeight(24)

nextButton:SetPoint(
    "BOTTOMRIGHT",
    frame,
    "BOTTOMRIGHT",
    -28,
    28
)

nextButton:SetText("Next")

local pageLabel = frame:CreateFontString(
    nil,
    "OVERLAY",
    "GameFontHighlight"
)

pageLabel:SetPoint(
    "BOTTOM",
    frame,
    "BOTTOM",
    0,
    35
)

pageLabel:SetText("Page 0 / 0")

local function SetQualityText(
    fontString,
    name,
    quality
)
    local color =
        ITEM_QUALITY_COLORS[quality]

    if color then
        fontString:SetTextColor(
            color.r,
            color.g,
            color.b
        )
    else
        fontString:SetTextColor(
            1,
            1,
            1
        )
    end

    fontString:SetText(name)
end

UpdateGetButton = function()
    if selectedItemEntry then
        getButton:Enable()
    else
        getButton:Disable()
    end
end

local function RefreshVisibleItemNames()
    for i = 1, PAGE_SIZE do
        local row = rows[i]
        local item = pendingItems[i]

        if item and
           row.itemEntry == item.itemEntry then

            local name =
                GetItemInfo(
                    item.itemEntry
                )

            if name then
                SetQualityText(
                    row.text,
                    name,
                    item.quality
                )
            end
        end
    end
end

local function RenderArchive()
    for i = 1, PAGE_SIZE do
        local row = rows[i]
        local item = pendingItems[i]

        row.selection:Hide()

        if item then
            row.itemEntry =
                item.itemEntry

            local name =
                GetItemInfo(
                    item.itemEntry
                )

            if not name then
                name =
                    "Item " ..
                    item.itemEntry
            end

            SetQualityText(
                row.text,
                name,
                item.quality
            )

            row:Show()
        else
            row.itemEntry = nil
            row.text:SetText("")
            row:Hide()
        end
    end

    ClearSelection()
    UpdateGetButton()

    countLabel:SetText(
        totalCount .. " items"
    )

    if totalPages > 0 then
        pageLabel:SetText(
            "Page " ..
            (currentPage + 1) ..
            " / " ..
            totalPages
        )
    else
        pageLabel:SetText(
            "Page 0 / 0"
        )
    end

    if currentPage <= 0 then
        previousButton:Disable()
    else
        previousButton:Enable()
    end

    if totalPages == 0 or
       currentPage >= totalPages - 1 then

        nextButton:Disable()
    else
        nextButton:Enable()
    end
end

previousButton:SetScript(
    "OnClick",
    function()
        if currentPage > 0 then
            statusLabel:SetText("")
            SendListRequest(
                currentPage - 1
            )
        end
    end
)

nextButton:SetScript(
    "OnClick",
    function()
        if currentPage + 1 < totalPages then
            statusLabel:SetText("")
            SendListRequest(
                currentPage + 1
            )
        end
    end
)

-- ------------------------------------------------------------
-- Server messages
-- ------------------------------------------------------------

local eventFrame =
    CreateFrame("Frame")

eventFrame:RegisterEvent(
    "CHAT_MSG_ADDON"
)

eventFrame:RegisterEvent(
    "GET_ITEM_INFO_RECEIVED"
)

eventFrame:SetScript(
    "OnEvent",
    function(
        self,
        event,
        prefix,
        message,
        channel,
        sender
    )
        if event == "GET_ITEM_INFO_RECEIVED" then
            if frame:IsShown() then
                RefreshVisibleItemNames()
            end

            return
        end

        if event ~= "CHAT_MSG_ADDON" then
            return
        end

        if prefix ~= PREFIX then
            return
        end

        local parts =
            Split(
                message,
                "|"
            )

        if parts[1] == "BEGIN" then
            pendingItems = {}

            totalCount =
                tonumber(parts[2]) or 0

            currentPage =
                tonumber(parts[3]) or 0

            totalPages =
                tonumber(parts[4]) or 0

            return
        end

        if parts[1] == "ITEM" then
            local itemEntry =
                tonumber(parts[2])

            local quality =
                tonumber(parts[3]) or 0

            if itemEntry then
                table.insert(
                    pendingItems,
                    {
                        itemEntry = itemEntry,
                        quality = quality
                    }
                )
            end

            return
        end

        if parts[1] == "END" then
            RenderArchive()
            return
        end

        if parts[1] == "COPY" then
            local result = parts[2]

            if result == "SUCCESS" then
                statusLabel:SetTextColor(
                    0.2,
                    1.0,
                    0.2
                )

                statusLabel:SetText(
                    "Archive copy created."
                )

                return
            end

            statusLabel:SetTextColor(
                1.0,
                0.3,
                0.3
            )

            if result == "NOT_UNLOCKED" then
                statusLabel:SetText(
                    "Item is not unlocked."
                )
                return
            end

            if result == "CANNOT_STORE" then
                statusLabel:SetText(
                    "Unable to store item."
                )
                return
            end

            if result == "ITEM_NOT_FOUND" then
                statusLabel:SetText(
                    "Item template not found."
                )
                return
            end

            if result == "CREATE_FAILED" then
                statusLabel:SetText(
                    "Failed to create item."
                )
                return
            end

            if result == "INVALID_REQUEST" then
                statusLabel:SetText(
                    "Invalid archive request."
                )
                return
            end

            statusLabel:SetText(
                "Archive request failed."
            )

            return
        end
    end
)

local function OpenArchive()
    statusLabel:SetTextColor(
        1,
        1,
        1
    )

    statusLabel:SetText("")

    frame:Show()

    SendListRequest(0)
end

SLASH_BOSSARCHIVE1 = "/bossarchive"
SLASH_BOSSARCHIVE2 = "/ba"

SlashCmdList["BOSSARCHIVE"] =
    function()
        if frame:IsShown() then
            frame:Hide()
        else
            OpenArchive()
        end
    end