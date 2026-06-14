-- LoginView.lua
-- Tela de login usando CPF + senha numerica

local M = {}
local cpfBuf = ""
local passBuf = ""
local currentUser = nil

function M.getCurrentUser()
	return currentUser
end

function M.logout()
	currentUser = nil
	cpfBuf = ""
	passBuf = ""
end

function M.render()
	local dm = getDataManager()
	if not dm then
		imgui.Text("DataManager nao inicializado")
		return
	end

	local sw, sh = 800, 600
	local pw, ph = 300, 200
	imgui.SetNextWindowPos((sw - pw) / 2, (sh - ph) / 2, "Always")
	imgui.SetNextWindowSize(pw, ph, "Always")

	if imgui.Begin("Login", nil, {"NoResize", "NoTitleBar"}) then
		imgui.PushStyleColor(imgui.Col_Text, 0xFF000000)
		imgui.PushStyleColor(imgui.Col_WindowBg, 0xFFFFFFFF)

		imgui.SetWindowFontScale(1.5)
		imgui.Text("AEDS III - Sistema de Matriculas")
		imgui.SetWindowFontScale(1.0)
		imgui.Separator()
		imgui.Spacing()

		imgui.Text("CPF:")
		imgui.SetNextItemWidth(-1)
		imgui.PushID("login_cpf")
		cpfBuf = imgui.InputText("##cpf", cpfBuf)
		imgui.PopID()

		imgui.Spacing()
		imgui.Text("Senha numerica:")
		imgui.SetNextItemWidth(-1)
		imgui.PushID("login_pass")
		passBuf = imgui.InputText("##pass", passBuf)
		imgui.PopID()

		imgui.Spacing()
		imgui.Spacing()

		if imgui.Button("Entrar", 150, 30) then
			local cpf = cpfBuf
			local pass = tonumber(passBuf)
			if cpf ~= "" and pass then
				local result = dm:login(cpf, pass)
				if result then
					currentUser = {
						id = result.userId,
						role = result.role,
						name = result.name
					}
				else
					imgui.OpenPopup("ErroLogin")
				end
			else
				imgui.OpenPopup("ErroLogin")
			end
		end

		if imgui.BeginPopupModal("ErroLogin", nil, {"NoResize"}) then
			imgui.Text("CPF ou senha invalidos")
			imgui.Spacing()
			if imgui.Button("OK", 80, 0) then
				imgui.CloseCurrentPopup()
			end
			imgui.EndPopup()
		end

		imgui.PopStyleColor(2)
		imgui.End()
	end
end

return M
