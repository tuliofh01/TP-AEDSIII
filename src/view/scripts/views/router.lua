-- router.scripts
-- Controlador de navegacao - carrega views e renderiza UI principal

local common = require("common")
local MainMenu = require("MainMenu")
local StudentCreate = require("StudentCreate")
local StudentList = require("StudentList")
local StudentDetail = require("StudentDetail")

-- Estado da view atual
local currentView = "menu"

-- Renderiza conteudo principal conforme view selecionada
local function renderContent()
	imgui.SetNextWindowPos(160, 0, "Always")
	imgui.SetNextWindowSize(640, 600, "Always")

	if imgui.Begin("Conteudo", nil, {"NoResize"}) then
		-- Fundo branco
		imgui.PushStyleColor(common.COL.WindowBg, common.COLORS.White)
		imgui.PushStyleColor(common.COL.Text, common.COLORS.Black)

		if currentView == "menu" then
			MainMenu.render()
		elseif currentView == "create" then
			StudentCreate.render()
		elseif currentView == "list" then
			StudentList.render()
		elseif currentView == "search" then
			StudentDetail.render()
		end

		imgui.PopStyleColor(2)
		imgui.End()
	end
end

-- Verifica se precisa reconstruir indice
local function checkRebuild()
	local dm = getDataManager()
	if not dm then return end

	if dm:needsRebuild() then
		imgui.OpenPopup("Rebuild")
	end

	if imgui.BeginPopupModal("Rebuild", nil, {"NoResize"}) then
		imgui.PushStyleColor(common.COL.Text, common.COLORS.White)
		imgui.Text("10 registros ativos. Deseja reconstruir o indice?")
		imgui.Spacing()
		imgui.SameLine()
		if common.button("Sim", common.COLORS.Green, 80, 0) then
			dm:triggerRebuild()
			imgui.CloseCurrentPopup()
		end
		imgui.SameLine()
		if common.button("Ignorar", common.COLORS.Red, 80, 0) then
			dm:ignoreRebuildForSession()
			imgui.CloseCurrentPopup()
		end
		imgui.PopStyleColor()
		imgui.EndPopup()
	end
end

-- Funcao global chamada pelo C++ a cada frame
function RenderUI()
	-- Sidebar
	currentView = common.sidebar(currentView)

	-- Conteudo principal
	renderContent()

	-- Verificacao de rebuild
	checkRebuild()
end
