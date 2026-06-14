-- router.lua
-- Controlador de navegacao com autenticacao

local common = require("handlers.common")
local LoginView = require("GUI.functional_views.LoginView")

local MainMenu = require("GUI.functional_views.MainMenu")
local StudentCreate = require("GUI.displayer_views.StudentCreate")
local StudentList = require("GUI.functional_views.StudentList")
local StudentDetail = require("GUI.displayer_views.StudentDetail")
local StudentProfile = require("GUI.displayer_views.StudentProfile")
local TeacherCreate = require("GUI.functional_views.TeacherCreate")
local TeacherList = require("GUI.functional_views.TeacherList")
local SubjectCreate = require("GUI.functional_views.SubjectCreate")
local SubjectList = require("GUI.functional_views.SubjectList")
local EnrollmentList = require("GUI.displayer_views.EnrollmentList")

local currentView = "menu"
local errorMsg = ""

local function renderContent(user)
	imgui.SetNextWindowPos(190, 0, "Always")
	imgui.SetNextWindowSize(610, 600, "Always")

	if imgui.Begin("Conteudo", nil, {"NoResize"}) then
		imgui.PushStyleColor(common.COL.WindowBg, common.COLORS.White)
		imgui.PushStyleColor(common.COL.Text, common.COLORS.Black)

		if user.role == "S" then
			-- Student views
			if currentView == "profile" then
				StudentProfile.render(user.id)
			elseif currentView == "myenr" then
				EnrollmentList.render(user.id, true)
			else
				StudentProfile.render(user.id)
			end
		else
			-- Teacher / Admin views
			if currentView == "menu" then
				MainMenu.render()
			elseif currentView == "create" then
				StudentCreate.render()
			elseif currentView == "list" then
				StudentList.render()
			elseif currentView == "search" then
				StudentDetail.render()
			elseif currentView == "tcreate" then
				TeacherCreate.render()
			elseif currentView == "tlist" then
				TeacherList.render()
			elseif currentView == "screate" then
				SubjectCreate.render()
			elseif currentView == "slist" then
				SubjectList.render()
			elseif currentView == "elist" then
				EnrollmentList.render(nil, false)
			else
				MainMenu.render()
			end
		end

		imgui.PopStyleColor(2)
		imgui.End()
	end
end

function RenderUI()
	local user = LoginView.getCurrentUser()
	if user == nil then
		LoginView.render()
		return
	end

	if common.navigateTo then
		currentView = common.navigateTo
		common.navigateTo = nil
	end

	currentView = common.sidebar(currentView, user)
	renderContent(user)
end
