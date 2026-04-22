-- common.lua
-- Componentes UI compartilhados - tema minimalista 4 cores
-- Preto (#000000), Branco (#FFFFFF), Vermelho (#FF0000), Verde (#00FF00)

local M = {}

-- Cores do tema como valores IM_COL32
M.COLORS = {
	Black  = 0xFF000000,
	White  = 0xFFFFFFFF,
	Red    = 0xFF0000FF,
	Green  = 0xFF00FF00,
}

-- Imagem inversa para PushStyleColor (ImGuiCol_* indices)
M.COL = {
	WindowBg  = imgui.Col_WindowBg or 20,
	ChildBg   = 21,
	Text      = imgui.Col_Text or 0,
	Button    = imgui.Col_Button or 27,
	ButtonHov = imgui.Col_ButtonHovered or 28,
	ButtonAct = imgui.Col_ButtonActive or 29,
	FrameBg   = imgui.Col_FrameBg or 24,
	Header    = imgui.Col_Header or 33,
	Separator = imgui.Col_Separator or 71,
}

-- Aplica cor por nome
function M.setColor(colIdx, colorVal)
	imgui.PushStyleColor(colIdx, colorVal)
end

function M.popColors(n)
	imgui.PopStyleColor(n or 1)
end

-- Botao estilizado com cor de fundo
function M.button(label, bgColor, w, h)
	local pushed = 0
	if bgColor then
		M.setColor(M.COL.Button, bgColor)
		M.setColor(M.COL.ButtonHov, bgColor)
		pushed = 2
	end
	local clicked = imgui.Button(label, w or 0, h or 0)
	if pushed > 0 then M.popColors(pushed) end
	return clicked
end

-- Texto com cor
function M.textColored(text, color)
	if color then M.setColor(M.COL.Text, color) end
	imgui.Text(text)
	if color then M.popColors() end
end

-- Barra lateral minimalista
function M.sidebar(currentView)
	imgui.SetNextWindowPos(0, 0, "Always")
	imgui.SetNextWindowSize(150, 600, "Always")

	if imgui.Begin("Nav", nil, {"NoResize", "NoTitleBar"}) then
		-- Fundo preto
		M.setColor(M.COL.WindowBg, M.COLORS.Black)
		M.setColor(M.COL.Text, M.COLORS.White)

		imgui.Text("AEDS III")
		imgui.Separator()
		imgui.Spacing()

		local views = {
			{ id = "menu",    label = "Menu" },
			{ id = "create",  label = "Cadastrar" },
			{ id = "list",    label = "Listar" },
			{ id = "search",  label = "Consultar" },
		}

		for _, v in ipairs(views) do
			local isActive = (currentView == v.id)
			local bg = isActive and M.COLORS.Green or M.COLORS.Black
			if M.button(v.label, bg, 130, 30) then
				currentView = v.id
			end
			imgui.Spacing()
		end

		imgui.Separator()
		imgui.Spacing()

		-- Info do manager
		local dm = getDataManager()
		if dm then
			local cnt = dm:getActiveCount()
			M.textColored("Ativos: " .. tostring(cnt), M.COLORS.Green)
		end

		M.popColors(2)
		imgui.End()
	end

	return currentView
end

-- Popup de erro
function M.errorPopup(msg)
	if msg and msg ~= "" then
		imgui.OpenPopup("Erro")
	end
	if imgui.BeginPopupModal("Erro", nil, {"NoResize"}) then
		M.textColored(msg, M.COLORS.Red)
		imgui.Spacing()
		if M.button("OK", M.COLORS.Red, 80, 0) then
			imgui.CloseCurrentPopup()
		end
		imgui.EndPopup()
	end
end

-- Popup de confirmacao (rebuild)
function M.confirmPopup(title, msg, onYes, onNo)
	if imgui.BeginPopupModal(title, nil, {"NoResize"}) then
		imgui.Text(msg)
		imgui.Spacing()
		imgui.SameLine()
		if M.button("Sim", M.COLORS.Green, 80, 0) then
			imgui.CloseCurrentPopup()
			if onYes then onYes() end
		end
		imgui.SameLine()
		if M.button("Nao", M.COLORS.Red, 80, 0) then
			imgui.CloseCurrentPopup()
			if onNo then onNo() end
		end
		imgui.EndPopup()
	end
end

return M
