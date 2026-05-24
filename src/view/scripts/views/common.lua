-- common.lua
-- Componentes UI compartilhados - tema minimalista 4 cores

local M = {}

M.COLORS = {
	Black  = 0xFF000000,
	White  = 0xFFFFFFFF,
	Red    = 0xFF0000FF,
	Green  = 0xFF00FF00,
}

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

function M.setColor(colIdx, colorVal)
	imgui.PushStyleColor(colIdx, colorVal)
end

function M.popColors(n)
	imgui.PopStyleColor(n or 1)
end

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

function M.textColored(text, color)
	if color then M.setColor(M.COL.Text, color) end
	imgui.Text(text)
	if color then M.popColors() end
end

function M.sidebar(currentView)
	imgui.SetNextWindowPos(0, 0, "Always")
	imgui.SetNextWindowSize(150, 600, "Always")

	if imgui.Begin("Nav", nil, {"NoResize", "NoTitleBar"}) then
		M.setColor(M.COL.WindowBg, M.COLORS.Black)
		M.setColor(M.COL.Text, M.COLORS.White)

		imgui.Text("AEDS III")
		imgui.Separator()
		imgui.Spacing()

		local sections = {
			{ label = "-- Estudantes --" },
			{ id = "create",  label = "Cadastrar" },
			{ id = "list",    label = "Listar" },
			{ id = "search",  label = "Consultar" },
			{ label = "-- Professores --" },
			{ id = "tcreate", label = "Cadastrar" },
			{ id = "tlist",   label = "Listar" },
			{ label = "-- Disciplinas --" },
			{ id = "screate", label = "Cadastrar" },
			{ id = "slist",   label = "Listar" },
			{ label = "-- Matriculas --" },
			{ id = "elist",   label = "Gerenciar" },
		}

		for _, v in ipairs(sections) do
			if v.label:sub(1, 2) == "--" then
				M.textColored(v.label, M.COLORS.Green)
				imgui.Spacing()
			else
				local isActive = (currentView == v.id)
				local bg = isActive and M.COLORS.Green or M.COLORS.Black
				if M.button(v.label, bg, 130, 25) then
					currentView = v.id
				end
				imgui.Spacing()
			end
		end

		imgui.Separator()
		imgui.Spacing()

		-- Info counts
		local dm = getDataManager()
		if dm then
			local sc = dm:getActiveCount('S')
			local tc = dm:getActiveCount('T')
			local bc = dm:getActiveCount('B')
			M.textColored("Alunos: " .. tostring(sc), M.COLORS.Green)
			M.textColored("Prof: " .. tostring(tc), M.COLORS.Green)
			M.textColored("Disc: " .. tostring(bc), M.COLORS.Green)
		end

		M.popColors(2)
		imgui.End()
	end

	return currentView
end

function M.errorPopup(msg)
	if msg and msg ~= "" then
		imgui.OpenPopup("Erro")
	end
	if imgui.BeginPopupModal("Erro", nil, {"NoResize"}) then
		if msg:find("sucesso") or msg:find("Sucesso") then
			M.textColored(msg, M.COLORS.Green)
		else
			M.textColored(msg, M.COLORS.Red)
		end
		imgui.Spacing()
		if M.button("OK", M.COLORS.Black, 80, 0) then
			imgui.CloseCurrentPopup()
		end
		imgui.EndPopup()
	end
end

return M
