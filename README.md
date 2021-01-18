# A DirectX 11 renderer

A framework for practicing graphics and game engine programming.

# Features

- ECS and Scene System
- Blinn-Phong Lighting Model
- Material System
- Scene Editor
- Logging

# Usage

All resources (models, textures, etc.) must be placed in Resources folder and all files of a model must be placed in the same parent folder in Resources.

It must be compiled in Release-Editor or Debug-Editor configuration to activate scene editor mode, in the editor mode free camera can be used while 
holding right mouse button and using W, A, S, D, Space (resets camera to default), mouse wheel (movement speed).

A scene can be initialized in Scene::Init.

```cpp
void Scene::Init()
{
	Scene::CreateNewScene(1, "first scene");
	
	Entity* sponza = Scene::GetScene(1)->NewEntity(Entity::FindPrefab("sponza.obj"));
	sponza->GetTransform()->SetLocalScale(0.01f, 0.01f, 0.01f);

	Entity* light = Scene::GetScene(1)->NewEntity("Light");
	light->AddComponent<PointLight>();

	Scene::LoadScene(1);
}
```

![Capture](https://user-images.githubusercontent.com/42220126/104954667-b686e080-59d9-11eb-8148-8d26f6a59133.PNG)

# 3rd Party Libraries

- [assimp](https://github.com/assimp/assimp)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [stb](https://github.com/nothings/stb)
