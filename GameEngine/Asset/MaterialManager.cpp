#include "MaterialManager.h"

#include <fstream>
#include <unordered_map>
#include <windows.h>

CMaterialManager::CMaterialManager()
{}

CMaterialManager::~CMaterialManager()
{}

static std::string GetExeDirA()
{
	char Buf[MAX_PATH] = {};
	GetModuleFileNameA(nullptr, Buf, MAX_PATH);
	char* Last = strrchr(Buf, '\\');
	if (!Last) Last = strrchr(Buf, '/');
	if (Last) *(Last + 1) = '\0';
	return std::string(Buf);
}

static std::wstring AnsiToWide(const std::string& s)
{
	if (s.empty()) return {};
	int Len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, nullptr, 0);
	if (Len <= 0) return {};
	std::wstring Result(Len - 1, 0);
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, &Result[0], Len);
	return Result;
}

bool CMaterialManager::Init()
{
	CreateMaterial("Default", "Material", "Linear");

	AutoLoadMaterials(GetExeDirA() + "Asset\\Material\\");

	return true;
}

void CMaterialManager::AutoLoadMaterials(const std::string& Dir)
{
	std::string Pattern = Dir + "*.mat";
	WIN32_FIND_DATAA FindData = {};
	HANDLE hFind = FindFirstFileA(Pattern.c_str(), &FindData);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do
	{
		std::string FilePath = Dir + FindData.cFileName;
		LoadMatFile(FilePath);
	}
	while (FindNextFileA(hFind, &FindData));

	FindClose(hFind);
}

void CMaterialManager::LoadMatFile(const std::string& FilePath)
{
	std::ifstream File(FilePath);
	if (!File.is_open())
		return;

	std::unordered_map<std::string, std::string> Props;
	std::string Line;
	while (std::getline(File, Line))
	{
		auto Eq = Line.find('=');
		if (Eq == std::string::npos) continue;
		Props[Line.substr(0, Eq)] = Line.substr(Eq + 1);
	}
	File.close();

	auto Get  = [&](const std::string& Key, const std::string& Def) -> std::string {
		auto It = Props.find(Key); return It != Props.end() ? It->second : Def;
	};
	auto GetF = [&](const std::string& Key, float Def) -> float {
		auto It = Props.find(Key);
		if (It == Props.end() || It->second.empty()) return Def;
		try { return std::stof(It->second); } catch (...) { return Def; }
	};

	std::string Name    = Get("Name",   "");
	std::string Shader  = Get("Shader", "Material");
	std::string Sampler = Get("Sampler","Linear");
	std::string Blend   = Get("Blend",  "(none)");
	std::string TexName = Get("TexName","");
	std::string TexPath = Get("TexPath","");
	float R = GetF("R", 1.f), G = GetF("G", 1.f), B = GetF("B", 1.f), A = GetF("A", 1.f);
	float Opacity = GetF("Opacity", 1.f);

	if (Name.empty()) return;

	// 이미 존재하면 덮어쓰지 않음
	if (!FindMaterial(Name).expired()) return;

	if (!CreateMaterial(Name, Shader, Sampler)) return;

	auto Mat = FindMaterial(Name).lock();
	if (!Mat) return;

	Mat->SetBaseColor(R, G, B, A);
	Mat->SetOpacity(Opacity);

	if (Blend != "(none)" && !Blend.empty())
		Mat->SetRenderState(Blend);

	if (!TexPath.empty() && !TexName.empty())
	{
		std::wstring WPath = AnsiToWide(TexPath);
		Mat->AddTextureFullPath(TexName, WPath.c_str());
	}
}

std::vector<std::string> CMaterialManager::GetAllMaterialNames() const
{
	std::vector<std::string> Names;
	for (auto& Pair : mMaterialMap)
	{
		const std::string& Key = Pair.first;
		if (Key.size() > 9 && Key[0] == 'M' && Key[8] == '_')
			Names.push_back(Key.substr(9));
		else
			Names.push_back(Key);
	}
	return Names;
}

bool CMaterialManager::CreateMaterial(const std::string& Name, const std::string& PixelShader, const std::string& SamplerName)
{
	if (!FindMaterial(Name).expired())
	{
		return false;
	}

	std::string MatName = "Material_" + Name;

	std::shared_ptr<CMaterial> Material = std::make_shared<CMaterial>();

	Material->SetName(MatName);
	Material->Init(PixelShader, SamplerName);

	mMaterialMap.insert(std::make_pair(MatName, Material));

	return true;
}

std::shared_ptr<CMaterial> CMaterialManager::CreateMaterialInstance(const std::string& Name)
{
	//현재 존재하는 머티리얼의 복사본을 반환한다.

	auto Material = FindMaterial(Name);

	if (Material.expired())
	{
		return std::shared_ptr<CMaterial>();
	}

	auto _Material = Material.lock();

	return std::shared_ptr<CMaterial>(_Material->Clone());
}

std::weak_ptr<CMaterial> CMaterialManager::FindMaterial(const std::string& Name)
{
	std::string MatName = "Material_" + Name;

	auto iter = mMaterialMap.find(MatName);

	if (iter == mMaterialMap.end())
	{
		return std::weak_ptr<CMaterial>();
	}

	return iter->second;
}

