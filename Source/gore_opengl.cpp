#include "gore_opengl.h"

enum texture_allocation_flag {
	TextureAllocation_NearestFiltering = 0x01,
	TextureAllocation_GenerateMipmaps = 0x02,
	TextureAllocation_EnableAnisotropic = 0x04,
};

GLuint OpenGLAllocateTexture(
	bitmap_info* Buffer, 
	u32 TextureAllocationFlag,
	gl_state* GLState) 
{
	GLuint TextureHandle;
	glGenTextures(1, &TextureHandle);

	glBindTexture(GL_TEXTURE_2D, TextureHandle);
	if (TextureAllocationFlag & TextureAllocation_EnableAnisotropic) {
		if (GLState->AnisotropicFilteringSupported) {
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, GLState->AnisotropicLevel);
		}
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (TextureAllocationFlag & TextureAllocation_NearestFiltering) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		Buffer->Width,
		Buffer->Height,
		0,
		GL_ABGR_EXT,
		GL_UNSIGNED_BYTE,
		Buffer->Pixels);


	glGenerateMipmap(GL_TEXTURE_2D);

	Buffer->TextureHandle = (void*)TextureHandle;

	glBindTexture(GL_TEXTURE_2D, 0);

	return(TextureHandle);
}

void OpenGLDeallocateTexture(bitmap_info* Buffer) {
	GLuint TextureHandle = (GLuint)Buffer->TextureHandle;

	glDeleteTextures(1, &TextureHandle);
	Buffer->TextureHandle = (void*)0;
}

GLuint OpenGLLoadProgramFromSource(char* VertexSource, char* FragmentSource) {
	char InfoLog[1024];
	int Success;
	
	GLuint VertexShader;
	GLuint FragmentShader;
	GLuint Program;

	VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShader, 1, &VertexSource, 0);
	glCompileShader(VertexShader);
	
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Success);
	if (!Success) {
		glGetShaderInfoLog(VertexShader, sizeof(InfoLog), 0, InfoLog);
		DEBUG_ERROR_LOG("Vert shader load error");
		//TODO(dima): Logging
	}

	FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, &FragmentSource, 0);
	glCompileShader(FragmentShader);

	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Success);
	if (!Success) {
		glGetShaderInfoLog(FragmentShader, sizeof(InfoLog), 0, InfoLog);
		DEBUG_ERROR_LOG(InfoLog);
		//TODO(dima): Logging
	}

	Program = glCreateProgram();
	glAttachShader(Program, VertexShader);
	glAttachShader(Program, FragmentShader);
	glLinkProgram(Program);

	glGetProgramiv(Program, GL_LINK_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(Program, sizeof(InfoLog), 0, InfoLog);
		DEBUG_ERROR_LOG(InfoLog);
		//TODO(dima): Logging
	}

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	return(Program);
}

gl_program OpenGLLoadShader(char* VertexPath, char* FragmentPath) {
	gl_program Result = {};

	platform_read_file_result VertexFile = PlatformApi.ReadFile(VertexPath);
	platform_read_file_result FragmentFile = PlatformApi.ReadFile(FragmentPath);

	Result.Handle = OpenGLLoadProgramFromSource(
		(char*)VertexFile.Data, 
		(char*)FragmentFile.Data);

	PlatformApi.FreeFileMemory(&VertexFile);
	PlatformApi.FreeFileMemory(&FragmentFile);

	return(Result);
}

gl_fxaa_shader OpenGLLoadFXAAShader() {
	gl_fxaa_shader Result = {};

	char* VertexPath = "../Data/Shaders/FXAAShader.vert";
	char* FragmentPath = "../Data/Shaders/FXAAShader.frag";

	Result.Program = OpenGLLoadShader(VertexPath, FragmentPath);

	Result.PosIndex = glGetAttribLocation(Result.Program.Handle, "inPos");
	Result.TexCoordIndex = glGetAttribLocation(Result.Program.Handle, "inTexCoords");

	Result.TextureLocation = glGetUniformLocation(Result.Program.Handle, "FramebufferTexture");
	Result.TextureSizeLocation = glGetUniformLocation(Result.Program.Handle, "FramebufferSize");

	return(Result);
}

gl_screen_shader OpenGLLoadScreenShader() {
	gl_screen_shader Result = {};

	char* VertexPath = "../Data/Shaders/ScreenShader.vert";
	char* FragmentPath = "../Data/Shaders/ScreenShader.frag";

	Result.Program = OpenGLLoadShader(VertexPath, FragmentPath);

	Result.PosIndex = glGetAttribLocation(Result.Program.Handle, "aPos");
	Result.TexIndex = glGetAttribLocation(Result.Program.Handle, "aTexCoords");

	Result.ScreenTextureLocation = glGetUniformLocation(Result.Program.Handle, "screenTexture");

	return(Result);
}

gl_wtf_shader OpenGLLoadWtfShader() {
	gl_wtf_shader Result = {};
	
	char* VertexPath = "../Data/Shaders/WtfShader.vert";
	char* FragmentPath = "../Data/Shaders/WtfShader.frag";

	Result.Program = OpenGLLoadShader(VertexPath, FragmentPath);

	Result.PositionIndex = glGetAttribLocation(Result.Program.Handle, "Position");
	Result.NormalIndex = glGetAttribLocation(Result.Program.Handle, "Normal");
	Result.UVIndex = glGetAttribLocation(Result.Program.Handle, "UV");
	Result.ColorIndex = glGetAttribLocation(Result.Program.Handle, "Color");
	Result.TangentIndex = glGetAttribLocation(Result.Program.Handle, "Tangent");

	Result.ModelMatrixLocation = glGetUniformLocation(Result.Program.Handle, "Model");
	Result.ViewMatrixLocation = glGetUniformLocation(Result.Program.Handle, "View");
	Result.ProjectionMatrixLocation = glGetUniformLocation(Result.Program.Handle, "Projection");
	Result.CameraPLocation = glGetUniformLocation(Result.Program.Handle, "CameraP");

	Result.SurfMatShineLocation = glGetUniformLocation(Result.Program.Handle, "Material.Shine");
	Result.SurfMatColorLocation = glGetUniformLocation(Result.Program.Handle, "Material.Color");

	Result.SurfMatDiffLocation = glGetUniformLocation(Result.Program.Handle, "Material.Diffuse");
	Result.SurfMatSpecLocation = glGetUniformLocation(Result.Program.Handle, "Material.Specular");
	Result.SurfMatEmisLocation = glGetUniformLocation(Result.Program.Handle, "Material.Emissive");

	Result.SurfMatHasDiffLocation = glGetUniformLocation(Result.Program.Handle, "Material.HasDiffuse");
	Result.SurfMatHasSpecLocation = glGetUniformLocation(Result.Program.Handle, "Material.HasSpecular");
	Result.SurfMatHasEmisLocation = glGetUniformLocation(Result.Program.Handle, "Material.HasEmissive");

	return(Result);
}

gl_voxel_shader OpenGLLoadVoxelShader() {
	gl_voxel_shader Result = {};

	char* VertexPath = "../Data/Shaders/VoxelShader.vert";
	char* FragmentPath = "../Data/Shaders/VoxelShader.frag";

	Result.Program = OpenGLLoadShader(VertexPath, FragmentPath);

	Result.VertexDataIndex = glGetAttribLocation(Result.Program.Handle, "VertexData");

	Result.ModelMatrixLocation = glGetUniformLocation(Result.Program.Handle, "Model");
	Result.ViewMatrixLocation = glGetUniformLocation(Result.Program.Handle, "View");
	Result.ProjectionMatrixLocation = glGetUniformLocation(Result.Program.Handle, "Projection");
	Result.CameraPLocation = glGetUniformLocation(Result.Program.Handle, "CameraP");
	Result.FogColorLocation = glGetUniformLocation(Result.Program.Handle, "FogColor");

	Result.DiffuseMapLocation = glGetUniformLocation(Result.Program.Handle, "DiffuseMap");

	Result.DirDirectionLocation = glGetUniformLocation(Result.Program.Handle, "DirLight.Direction");
	Result.DirDiffuseLocation = glGetUniformLocation(Result.Program.Handle, "DirLight.Diffuse");
	Result.DirAmbientLocation = glGetUniformLocation(Result.Program.Handle, "DirLight.Ambient");

	return(Result);
}


void OpenGLUniformSurfaceMaterial(gl_state* GLState, render_state* State, gl_wtf_shader* Shader, surface_material* Mat) {
	glUniform1f(Shader->SurfMatShineLocation, Mat->Shine);
	glUniform3f(Shader->SurfMatColorLocation, Mat->Color.x, Mat->Color.y, Mat->Color.z);

	b32 HasDiffuseBitmap = (Mat->Diffuse || Mat->DiffuseInfo);
	b32 HasSpecularBitmap = (Mat->Specular || Mat->SpecularInfo);
	b32 HasEmissiveBitmap = (Mat->Emissive || Mat->EmissiveInfo);

	glUniform1i(Shader->SurfMatHasDiffLocation, HasDiffuseBitmap ? 1 : 0);
	if (HasDiffuseBitmap) {
		bitmap_info* Info = Mat->DiffuseInfo;
		if (Mat->Diffuse) {
			Info = GetBitmapFromID(State->AssetSystem, Mat->Diffuse);
		}

		if (Info) {
			if (!Info->TextureHandle) {
				OpenGLAllocateTexture(Info, TextureAllocation_EnableAnisotropic, GLState);
			}
		}

		_glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (GLuint)Info->TextureHandle);
		glUniform1i(Shader->SurfMatDiffLocation, 0);
	}

	glUniform1i(Shader->SurfMatHasSpecLocation, HasSpecularBitmap ? 1 : 0);
	if (HasSpecularBitmap) {
		bitmap_info* Info = Mat->SpecularInfo;
		if (Mat->Specular) {
			Info = GetBitmapFromID(State->AssetSystem, Mat->Specular);
		}

		if (Info) {
			if (!Info->TextureHandle) {
				OpenGLAllocateTexture(Info, TextureAllocation_EnableAnisotropic, GLState);
			}
		}

		_glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, (GLuint)Info->TextureHandle);
		glUniform1i(Shader->SurfMatSpecLocation, 1);
	}

	glUniform1i(Shader->SurfMatHasEmisLocation, HasEmissiveBitmap ? 1 : 0);
	if (HasEmissiveBitmap) {
		bitmap_info* Info = Mat->EmissiveInfo;
		
		if (Mat->Emissive) {
			Info = GetBitmapFromID(State->AssetSystem, Mat->Emissive);
		}

		if (Info) {
			if (!Info->TextureHandle) {
				OpenGLAllocateTexture(Info, TextureAllocation_EnableAnisotropic, GLState);
			}
		}

		_glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, (GLuint)Info->TextureHandle);
		glUniform1i(Shader->SurfMatEmisLocation, 2);
	}
}

inline void OpenGLUseProgramBegin(gl_program* Program) {
	glUseProgram(Program->Handle);
}

inline void OpenGLUseProgramEnd() {
	glUseProgram(0);
}

void OpenGLRenderBitmap(gl_state* GLState, bitmap_info* Buffer, v2 P, v2 Dim, v4 Color = V4(1.0f, 1.0f, 1.0f, 1.0f)) {

	rect2 Rect = Rect2MinDim(P, Dim);

	if (!Buffer->TextureHandle) {
		OpenGLAllocateTexture(Buffer, 0, GLState);
	}

	glBindTexture(GL_TEXTURE_2D, (size_t)Buffer->TextureHandle);
	glBegin(GL_TRIANGLES);

	glColor4f(Color.r, Color.g, Color.b, Color.a);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(Rect.Min.x, Rect.Min.y);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(Rect.Max.x, Rect.Min.y);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(Rect.Max.x, Rect.Max.y);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(Rect.Min.x, Rect.Min.y);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(Rect.Max.x, Rect.Max.y);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(Rect.Min.x, Rect.Max.y);

	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLRenderRectangle(rect2 Rect, v4 Color) {
	glBegin(GL_TRIANGLES);

	glColor4f(Color.r, Color.g, Color.b, Color.a);

	glVertex2f(Rect.Min.x, Rect.Min.y);
	glVertex2f(Rect.Max.x, Rect.Min.y);
	glVertex2f(Rect.Max.x, Rect.Max.y);

	glVertex2f(Rect.Min.x, Rect.Min.y);
	glVertex2f(Rect.Max.x, Rect.Max.y);
	glVertex2f(Rect.Min.x, Rect.Max.y);

	glEnd();
}

void OpenGLSetScreenspace(int Width, int Height) {
	
	float a = 2.0f / (float)Width;
	float b = -2.0f / (float)Height;

	float c = 2.0f / 10000.0f;

	float ProjMatrix[] = {
		a, 0, 0, 0,
		0, b, 0, 0,
		0, 0, c, 0,
		-1.0f, 1.0f, -1.0f, 1.0f
	};

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(ProjMatrix);
}

void OpenGLRenderStackToOutput(gl_state* GLState, render_state* RenderState) {
	FUNCTION_TIMING();

	glEnable(GL_MULTISAMPLE);

	GLuint LastWriteFBO = GLState->FramebufferInitial.FBO;
	glBindFramebuffer(GL_FRAMEBUFFER, LastWriteFBO);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	//glEnable(GL_CULL_FACE);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	OpenGLSetScreenspace(RenderState->RenderWidth, RenderState->RenderHeight);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//NOTE(dima): Iteration through render stack
	u8* At = (u8*)RenderState->Data.BaseAddress;
	u8* StackEnd = (u8*)RenderState->Data.BaseAddress + RenderState->Data.Used;

	game_camera_setup* CameraSetup = &RenderState->CameraSetup;

	glUseProgram(GLState->WtfShader.Program.Handle);
	glUniformMatrix4fv(GLState->WtfShader.ProjectionMatrixLocation, 1, GL_TRUE, CameraSetup->ProjectionMatrix.E);
	glUniformMatrix4fv(GLState->WtfShader.ViewMatrixLocation, 1, GL_TRUE, CameraSetup->ViewMatrix.E);
	glUniform3f(
		GLState->WtfShader.CameraPLocation,
		RenderState->CameraSetup.Camera.Position.x,
		RenderState->CameraSetup.Camera.Position.y,
		RenderState->CameraSetup.Camera.Position.z);
	glUseProgram(0);

	glUseProgram(GLState->VoxelShader.Program.Handle);
	glUniformMatrix4fv(GLState->VoxelShader.ProjectionMatrixLocation, 1, GL_TRUE, CameraSetup->ProjectionMatrix.E);
	glUniformMatrix4fv(GLState->VoxelShader.ViewMatrixLocation, 1, GL_TRUE, CameraSetup->ViewMatrix.E);
	glUniform3f(
		GLState->VoxelShader.CameraPLocation,
		RenderState->CameraSetup.Camera.Position.x,
		RenderState->CameraSetup.Camera.Position.y,
		RenderState->CameraSetup.Camera.Position.z);
	glUseProgram(0);

	font_info* CurrentFontInfo = 0;

	while (At < StackEnd) {
		render_stack_entry_header* Header = (render_stack_entry_header*)At;

		u32 SizeOfEntryType = Header->SizeOfEntryType;

		At += sizeof(render_stack_entry_header);
		switch (Header->Type) {
			case(RenderEntry_Bitmap): {
				render_stack_entry_bitmap* EntryBitmap = (render_stack_entry_bitmap*)At;

				OpenGLRenderBitmap(GLState, EntryBitmap->Bitmap, EntryBitmap->P, EntryBitmap->Dim, EntryBitmap->ModulationColor);
			}break;

			case(RenderEntry_Clear): {
				render_stack_entry_clear* EntryClear = (render_stack_entry_clear*)At;

				glClearColor(
					EntryClear->Color.r,
					EntryClear->Color.g,
					EntryClear->Color.b,
					1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}break;

			case(RenderEntry_Gradient): {
				render_stack_entry_gradient* EntryGrad = (render_stack_entry_gradient*)At;
					
				//TODO(DIMA): 
			}break;

			case(RenderEntry_Rectangle): {
				render_stack_entry_rectangle* EntryRect = (render_stack_entry_rectangle*)At;

				OpenGLRenderRectangle(Rect2MinDim(EntryRect->P, EntryRect->Dim), EntryRect->ModulationColor);
			}break;

			case RenderEntry_Glyph: {
				render_stack_entry_glyph* EntryGlyph = (render_stack_entry_glyph*)At;

				if (CurrentFontInfo) {
					glyph_info* Glyph = &CurrentFontInfo->Glyphs[CurrentFontInfo->CodepointToGlyphMapping[EntryGlyph->Codepoint]];

					v4 Color = EntryGlyph->ModulationColor;
					rect2 Rect = Rect2MinDim(EntryGlyph->P, EntryGlyph->Dim);
					v2 MinUV = Glyph->AtlasMinUV;
					v2 MaxUV = Glyph->AtlasMaxUV;

					glColor4f(Color.r, Color.g, Color.b, Color.a);

					float Depth = 10000.0f;
					
					glTexCoord2f(MinUV.x, MinUV.y);
					glVertex3f(Rect.Min.x, Rect.Min.y, Depth);
					glTexCoord2f(MaxUV.x, MinUV.y);
					glVertex3f(Rect.Max.x, Rect.Min.y, Depth);
					glTexCoord2f(MaxUV.x, MaxUV.y);
					glVertex3f(Rect.Max.x, Rect.Max.y, Depth);

					glTexCoord2f(MinUV.x, MinUV.y);
					glVertex3f(Rect.Min.x, Rect.Min.y, Depth);
					glTexCoord2f(MaxUV.x, MaxUV.y);
					glVertex3f(Rect.Max.x, Rect.Max.y, Depth);
					glTexCoord2f(MinUV.x, MaxUV.y);
					glVertex3f(Rect.Min.x, Rect.Max.y, Depth);
				}
			}break;

			case RenderEntry_BeginText: {
				render_stack_entry_begin_text* EntryBeginText = (render_stack_entry_begin_text*)At;

				CurrentFontInfo = EntryBeginText->FontInfo;
				bitmap_info* Buffer = &CurrentFontInfo->FontAtlasImage;

				if (!Buffer->TextureHandle) {
					OpenGLAllocateTexture(Buffer, 0, GLState);
				}
				glBindTexture(GL_TEXTURE_2D, (GLuint)Buffer->TextureHandle);
				glBegin(GL_TRIANGLES);

			}break;

			case RenderEntry_EndText: {
				render_stack_entry_end_text* EntryEndText = (render_stack_entry_end_text*)At;
				
				CurrentFontInfo = 0;

				glEnd();
				glBindTexture(GL_TEXTURE_2D, 0);
			}break;

			case RenderEntry_Mesh: {
				render_stack_entry_mesh* EntryMesh = (render_stack_entry_mesh*)At;

				mesh_info* MeshInfo = EntryMesh->MeshInfo;

				gl_wtf_shader* Shader = &GLState->WtfShader;

				if (!MeshInfo->Handle) {
					GLuint EBO, VBO, VAO;

					glGenVertexArrays(1, &VAO);
					glGenBuffers(1, &VBO);
					glGenBuffers(1, &EBO);

					/*
						NOTE(dima): We need to be accurate here
						because of the alignment..
					*/
					u32 OneVertexSize = sizeof(vertex_info);

					glBindVertexArray(VAO);

					glBindBuffer(GL_ARRAY_BUFFER, VBO);
					glBufferData(
						GL_ARRAY_BUFFER, 
						MeshInfo->VerticesCount * OneVertexSize, 
						MeshInfo->Vertices, 
						GL_STATIC_DRAW);

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
					glBufferData(
						GL_ELEMENT_ARRAY_BUFFER, 
						MeshInfo->IndicesCount * sizeof(u32), 
						MeshInfo->Indices, 
						GL_STATIC_DRAW);

					if (OpenGLArrayIsValid(Shader->PositionIndex)) {
						glEnableVertexAttribArray(Shader->PositionIndex);
						u32 POffset = offsetof(vertex_info, P);
						glVertexAttribPointer(Shader->PositionIndex, 3, GL_FLOAT, 0, OneVertexSize, (void*)POffset);
					}

					if (OpenGLArrayIsValid(Shader->NormalIndex)) {
						glEnableVertexAttribArray(Shader->NormalIndex);
						u32 NOffset = offsetof(vertex_info, N);
						glVertexAttribPointer(Shader->NormalIndex, 3, GL_FLOAT, 0, OneVertexSize, (void*)NOffset);
					}

					if (OpenGLArrayIsValid(Shader->UVIndex)) {
						glEnableVertexAttribArray(Shader->UVIndex);
						u32 UVOffset = offsetof(vertex_info, UV);
						glVertexAttribPointer(Shader->UVIndex, 2, GL_FLOAT, 0, OneVertexSize, (void*)UVOffset);
					}

					if (OpenGLArrayIsValid(Shader->TangentIndex)) {
						glEnableVertexAttribArray(Shader->TangentIndex);
						u32 TOffset = offsetof(vertex_info, T);
						glVertexAttribPointer(Shader->TangentIndex, 3, GL_FLOAT, 0, OneVertexSize, (void*)TOffset);
					}

					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glBindVertexArray(0);

					MeshInfo->Handle = (void*)VAO;
				}

				glUseProgram(GLState->WtfShader.Program.Handle);

				glEnable(GL_DEPTH_TEST);

				OpenGLUniformSurfaceMaterial(GLState, RenderState, &GLState->WtfShader, &EntryMesh->Material);

				glBindVertexArray((GLuint)MeshInfo->Handle);
				glUniformMatrix4fv(GLState->WtfShader.ModelMatrixLocation, 1, GL_TRUE, EntryMesh->TransformMatrix.E);
				glDrawElements(GL_TRIANGLES, MeshInfo->IndicesCount, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				glDisable(GL_DEPTH_TEST);

				glUseProgram(0);
			}break;

			case RenderEntry_VoxelMesh: {
				render_stack_entry_voxel_mesh* EntryVoxelMesh = (render_stack_entry_voxel_mesh*)At;

				gl_voxel_shader* Shader = &GLState->VoxelShader;
				voxel_mesh_info* Mesh = EntryVoxelMesh->MeshInfo;

				mat4 ModelTransform = Translate(Identity(), EntryVoxelMesh->P);

				if (Mesh->State == VoxelMeshState_Generated) {

					u32 TextureToBind = 0;
					if (EntryVoxelMesh->VoxelAtlasBitmap) {
						if (EntryVoxelMesh->VoxelAtlasBitmap->TextureHandle) {
							TextureToBind = (u32)EntryVoxelMesh->VoxelAtlasBitmap->TextureHandle;
						}
						else {
							TextureToBind = OpenGLAllocateTexture(
								EntryVoxelMesh->VoxelAtlasBitmap,
								TextureAllocation_NearestFiltering | TextureAllocation_EnableAnisotropic,
								GLState);
						}
					}

					if (!Mesh->MeshHandle) {
						BeginMutexAccess(&Mesh->MeshUseMutex);
						GLuint MeshVAO;
						GLuint MeshVBO;

						glGenVertexArrays(1, &MeshVAO);
						glGenBuffers(1, &MeshVBO);

						glBindVertexArray(MeshVAO);

						glBindBuffer(GL_ARRAY_BUFFER, MeshVBO);
						glBufferData(GL_ARRAY_BUFFER,
							Mesh->VerticesCount * sizeof(u32),
							&Mesh->Vertices[0], GL_DYNAMIC_DRAW);

						if (OpenGLArrayIsValid(Shader->VertexDataIndex)) {
							glEnableVertexAttribArray(Shader->VertexDataIndex);
							glVertexAttribPointer(Shader->VertexDataIndex, 1, GL_FLOAT, GL_FALSE, 4, 0);
						}

						glBindBuffer(GL_ARRAY_BUFFER, 0);
						glBindVertexArray(0);

						Mesh->MeshHandle = (void*)MeshVAO;
						Mesh->MeshHandle2 = (void*)MeshVBO;
						EndMutexAccess(&Mesh->MeshUseMutex);
					}

					glUseProgram((u32)Shader->Program.Handle);

					glEnable(GL_DEPTH_TEST);

					_glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, TextureToBind);
					glUniform1i(Shader->DiffuseMapLocation, 0);

					glUniformMatrix4fv(
						Shader->ModelMatrixLocation, 1, GL_TRUE, ModelTransform.E);

					glBindVertexArray((u32)Mesh->MeshHandle);
					glDrawArrays(GL_TRIANGLES, 0, Mesh->VerticesCount);
					glBindVertexArray(0);

					glDisable(GL_DEPTH_TEST);

					glUseProgram(0);
				}
			}break;

			case RenderEntry_Lighting: {
				gl_wtf_shader* WtfShader = &GLState->WtfShader;

				glUseProgram(WtfShader->Program.Handle);

				glUseProgram(0);
			}break;

			case RenderEntry_VoxelLighting: {
				render_stack_entry_voxel_lighting* Entry = (render_stack_entry_voxel_lighting*)At;

				gl_voxel_shader* VoxelShader = &GLState->VoxelShader;

				v3 VoxDirDir = V3(0.5f, -0.5f, 0.5f);
				v3 VoxDirAmb = V3(0.1f, 0.1f, 0.1f);
				v3 VoxDirDif = V3(1.0f, 1.0f, 1.0f);

				glUseProgram(VoxelShader->Program.Handle);
				glUniform3f(
					VoxelShader->FogColorLocation,
					Entry->FogColor.r,
					Entry->FogColor.g,
					Entry->FogColor.b);
				glUniform3f(
					VoxelShader->DirDirectionLocation, 
					VoxDirDir.x, 
					VoxDirDir.y, 
					VoxDirDir.z);
				glUniform3f(
					VoxelShader->DirDiffuseLocation,  
					VoxDirDif.x, 
					VoxDirDif.y, 
					VoxDirDif.z);
				glUniform3f(
					VoxelShader->DirAmbientLocation,
					VoxDirAmb.x, 
					VoxDirAmb.y, 
					VoxDirAmb.z);
				glUseProgram(0);

			}break;

			case RenderEntry_Test: {

			}break;

			default: {
				Assert(!"Invalid entry type");
			}break;
		}

		At += Header->SizeOfEntryType;
	}

	glDisable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//NOTE(dima): FXAA antialiasing
	if (GLState->AntialiasingType == AA_FXAA) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, LastWriteFBO);
		LastWriteFBO = GLState->FramebufferFXAA.FBO;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, LastWriteFBO);

		glBlitFramebuffer(
			0, 0,
			RenderState->RenderWidth,
			RenderState->RenderHeight,
			0, 0,
			RenderState->RenderWidth,
			RenderState->RenderHeight,
			GL_COLOR_BUFFER_BIT,
			GL_NEAREST);

		glBindFramebuffer(GL_FRAMEBUFFER, GLState->FramebufferFXAA.FBO);

		OpenGLUseProgramBegin(&GLState->FXAAShader.Program);
		glBindVertexArray(GLState->ScreenQuadVAO);
		_glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GLState->FramebufferFXAA.Texture);
		glUniform1i(GLState->FXAAShader.TextureLocation, 0);
		glUniform2f(
			GLState->FXAAShader.TextureSizeLocation,
			RenderState->RenderWidth,
			RenderState->RenderHeight);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		OpenGLUseProgramEnd();
	}

	//NOTE(dima): Finalizing screen shader
	glBindFramebuffer(GL_READ_FRAMEBUFFER, LastWriteFBO);
	LastWriteFBO = GLState->FramebufferResult.FBO;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, LastWriteFBO);
	glBlitFramebuffer(
		0, 0,
		RenderState->RenderWidth,
		RenderState->RenderHeight,
		0, 0,
		RenderState->RenderWidth,
		RenderState->RenderHeight,
		GL_COLOR_BUFFER_BIT,
		GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	OpenGLUseProgramBegin(&GLState->ScreenShader.Program);
	glBindVertexArray(GLState->ScreenQuadVAO);
	_glActiveTexture(GL_TEXTURE0);
	glUniform1i(GLState->ScreenShader.ScreenTextureLocation, 0);
	glBindTexture(GL_TEXTURE_2D, GLState->FramebufferResult.Texture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	OpenGLUseProgramEnd();

	//glDeleteFramebuffers(1, &FramebufferObject);
	//glDeleteRenderbuffers(1, &DepthStencilRBO);
}

static void OpenGLInitMultisampleFramebuffer(
	opengl_framebuffer* Framebuffer,
	int RenderWidth,
	int RenderHeight,
	int NumberOfSamples)
{
	//NOTE(dima): Initializing of multisample framebuffer
	glGenFramebuffers(1, &Framebuffer->FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer->FBO);

	glGenTextures(1, &Framebuffer->Texture);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, Framebuffer->Texture);
	glTexImage2DMultisample(
		GL_TEXTURE_2D_MULTISAMPLE,
		NumberOfSamples,
		GL_RGBA8,
		RenderWidth,
		RenderHeight,
		GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D_MULTISAMPLE,
		Framebuffer->Texture,
		0);

	glGenRenderbuffers(1, &Framebuffer->DepthStencilRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, Framebuffer->DepthStencilRBO);
	glRenderbufferStorageMultisample(
		GL_RENDERBUFFER,
		NumberOfSamples,
		GL_DEPTH24_STENCIL8,
		RenderWidth,
		RenderHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(
		GL_FRAMEBUFFER,
		GL_DEPTH_STENCIL_ATTACHMENT,
		GL_RENDERBUFFER,
		Framebuffer->DepthStencilRBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		Assert(!"Framebuffer should be complete");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void OpenGLInitFramebuffer(
	opengl_framebuffer* Framebuffer,
	int RenderWidth,
	int RenderHeight)
{
	//NOTE(dima): Initializing internal framebuffer
	glGenFramebuffers(1, &Framebuffer->FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer->FBO);

	glGenTextures(1, &Framebuffer->Texture);
	glBindTexture(GL_TEXTURE_2D, Framebuffer->Texture);
	glTexImage2D(
		GL_TEXTURE_2D, 0,
		GL_RGBA,
		RenderWidth,
		RenderHeight,
		0,
		GL_ABGR_EXT,
		GL_UNSIGNED_BYTE,
		0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		Framebuffer->Texture,
		0);

	glGenRenderbuffers(1, &Framebuffer->DepthStencilRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, Framebuffer->DepthStencilRBO);
	glRenderbufferStorage(
		GL_RENDERBUFFER,
		GL_DEPTH24_STENCIL8,
		RenderWidth,
		RenderHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(
		GL_FRAMEBUFFER,
		GL_DEPTH_STENCIL_ATTACHMENT,
		GL_RENDERBUFFER,
		Framebuffer->DepthStencilRBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		Assert(!"Framebuffer should be complete");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static b32 OpenGLExtensionIsSupported(
	char* ExtensionName)
{
	b32 Result = 0;

	int ExtensionsCount;
	glGetIntegerv(GL_NUM_EXTENSIONS, &ExtensionsCount);

	for (int ExtensionIndex = 0;
		ExtensionIndex < ExtensionsCount;
		ExtensionIndex++)
	{
		const GLubyte* Extension = glGetStringi(GL_EXTENSIONS, ExtensionIndex);

		if (StringsAreEqual((char*)Extension, ExtensionName)) {
			Result = 1;
			break;
		}
	}

	return(Result);
}

void OpenGLInitState(
	gl_state* State, 
	int RenderWidth, 
	int RenderHeight) 
{
	*State = {};

	State->WtfShader = OpenGLLoadWtfShader();
	State->VoxelShader = OpenGLLoadVoxelShader();
	State->ScreenShader = OpenGLLoadScreenShader();
	State->FXAAShader = OpenGLLoadFXAAShader();

	//NOTE(dima): Checking for OpenGL extensions support
	//TODO(dima): Load parameters from game_settings
	State->AnisotropicFilteringSupported = OpenGLExtensionIsSupported("GL_EXT_texture_filter_anisotropic");
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &State->MaxAnisotropicLevel);
	State->AnisotropicLevelType = AnisoLevel_8x;
	State->AnisotropicLevel = GetAnisoLevelBasedOnParams(
		State->AnisotropicLevelType,
		State->MaxAnisotropicLevel);

	State->AntialiasingType = AA_FXAA;

	State->MultisamplingSupported =
		OpenGLExtensionIsSupported("GL_ARB_multisample") ||
		OpenGLExtensionIsSupported("GLX_ARB_multisample") ||
		OpenGLExtensionIsSupported("WGL_ARB_multisample");
	glGetIntegerv(GL_MAX_SAMPLES, &State->MaxMultisampleLevel);
	if (AntialiasingIsMSAA(State->AntialiasingType)) {
		State->MultisampleLevel = GetMSAALevel(State->AntialiasingType);
	}

	//NOTE(dima): Initializing of screen quad. Pos + Tex
#if 1
	float ScreenQuadFloats[] = {
		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,

		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
	};
#else
	float ScreenQuadFloats[] = {
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
	};
#endif

	glGenVertexArrays(1, &State->ScreenQuadVAO);
	glGenBuffers(1, &State->ScreenQuadVBO);

	glBindVertexArray(State->ScreenQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, State->ScreenQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ScreenQuadFloats), ScreenQuadFloats, GL_STATIC_DRAW);
	if (OpenGLArrayIsValid(State->ScreenShader.PosIndex)) {
		glEnableVertexAttribArray(State->ScreenShader.PosIndex);
		glVertexAttribPointer(State->ScreenShader.PosIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	}
	if (OpenGLArrayIsValid(State->ScreenShader.TexIndex)) {
		glEnableVertexAttribArray(State->ScreenShader.TexIndex);
		glVertexAttribPointer(State->ScreenShader.TexIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	}

	if (OpenGLArrayIsValid(State->FXAAShader.PosIndex)) {
		glEnableVertexAttribArray(State->FXAAShader.PosIndex);
		glVertexAttribPointer(State->FXAAShader.PosIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	}
	if (OpenGLArrayIsValid(State->FXAAShader.TexCoordIndex)) {
		glEnableVertexAttribArray(State->FXAAShader.TexCoordIndex);
		glVertexAttribPointer(State->FXAAShader.TexCoordIndex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	}
	glBindVertexArray(0);

	//NOTE(dima): Initialization of framebuffer objects
	if (State->MultisamplingSupported && AntialiasingIsMSAA(State->AntialiasingType)) {
		OpenGLInitMultisampleFramebuffer(&State->FramebufferInitial, RenderWidth, RenderHeight, State->MultisampleLevel);
	}
	else {
		OpenGLInitFramebuffer(&State->FramebufferInitial, RenderWidth, RenderHeight);
	}
	
	OpenGLInitFramebuffer(&State->FramebufferResult, RenderWidth, RenderHeight);
	OpenGLInitFramebuffer(&State->FramebufferFXAA, RenderWidth, RenderHeight);
}

void OpenGLProcessAllocationQueue() {

	FUNCTION_TIMING();

	dealloc_queue_entry* FirstEntry = 0;
	dealloc_queue_entry* LastEntry = 0;

	BeginMutexAccess(&PlatformApi.DeallocQueueMutex);
	
	//NOTE(dima): If there is something to allocate...
	if (PlatformApi.FirstUseAllocQueueEntry->Next != PlatformApi.FirstUseAllocQueueEntry) 
	{
		FirstEntry = PlatformApi.FirstUseAllocQueueEntry->Next;
		LastEntry = PlatformApi.FirstUseAllocQueueEntry->Prev;

#if 0
		PlatformApi.FirstUseAllocQueueEntry->Next = PlatformApi.FirstUseAllocQueueEntry;
		PlatformApi.FirstUseAllocQueueEntry->Prev = PlatformApi.FirstUseAllocQueueEntry;

 		//FirstEntry->Prev->Next = PlatformApi.FirstUseAllocQueueEntry;
		//LastEntry->Next->Prev = PlatformApi.FirstUseAllocQueueEntry;
		
		FirstEntry->Prev = PlatformApi.FirstFreeAllocQueueEntry;
		LastEntry->Next = PlatformApi.FirstFreeAllocQueueEntry->Next;
		
		FirstEntry->Prev->Next = FirstEntry;
		LastEntry->Next->Prev = LastEntry;
#endif
	}

	EndMutexAccess(&PlatformApi.DeallocQueueMutex);

	for (dealloc_queue_entry* Entry = FirstEntry;
		Entry;)
	{
		dealloc_queue_entry* NextEntry = Entry->Next;

		switch (Entry->EntryType) {
			case DeallocQueueEntry_Bitmap: {

			}break;

			case DeallocQueueEntry_VoxelMesh: {
				GLuint MeshVBO = (GLuint)Entry->Data.VoxelMeshData.Handle2;
				GLuint MeshVAO = (GLuint)Entry->Data.VoxelMeshData.Handle1;

				glDeleteVertexArrays(1, &MeshVAO);
				glDeleteBuffers(1, &MeshVBO);
			}break;
		}

#if 1
		BeginMutexAccess(&PlatformApi.DeallocQueueMutex);
		
		Entry->Next->Prev = Entry->Prev;
		Entry->Prev->Next = Entry->Next;

		Entry->Next = PlatformApi.FirstFreeAllocQueueEntry->Next;
		Entry->Prev = PlatformApi.FirstFreeAllocQueueEntry;

		Entry->Next->Prev = Entry;
		Entry->Prev->Next = Entry;

		EndMutexAccess(&PlatformApi.DeallocQueueMutex);
#endif

		if (Entry == LastEntry) {
			break;
		}

		Entry = NextEntry;
	}
}
//