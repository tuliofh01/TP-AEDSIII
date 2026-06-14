-- router.lua
-- Controlador de navegacao - carrega views e renderiza UI principal

local common = require("common")
local MainMenu = require("MainMenu")
local StudentCreate = require("StudentCreate")
local StudentList = require("StudentList")
local StudentDetail = require("StudentDetail")
local TeacherCreate = require("TeacherCreate")
local TeacherList = require("TeacherList")
local SubjectCreate = require("SubjectCreate")
local SubjectList = require("SubjectList")
local EnrollmentList = require("EnrollmentList")

local currentView = "menu"
local errorMsg = ""

local function renderContent()
	imgui.SetNextWindowPos(160, 0, "Always")
	imgui.SetNextWindowSize(640, 600, "Always")

	if imgui.Begin("Conteudo", nil, {"NoResize"}) then
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
		elseif currentView == "tcreate" then
			TeacherCreate.render()
		elseif currentView == "tlist" then
			TeacherList.render()
		elseif currentView == "screate" then
			SubjectCreate.render()
		elseif currentView == "slist" then
			SubjectList.render()
		elseif currentView == "elist" then
			EnrollmentList.render()
		end

		imgui.PopStyleColor(2)
		imgui.End()
	end
end

function RenderUI()
	currentView = common.sidebar(currentView)
	renderContent()
end
