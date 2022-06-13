#ifndef HVH_WC_GUI_GUILAYER_H
#define HVH_WC_GUI_GUILAYER_H

#include "font.h"
#include "text.h"
#include "text_wrap.h"
#include "dialogue.h"
#include "graphics/shader.h"
#include "sys/input.h"

class GuiLayer
{
public:
	GuiLayer();

	void initShaders()
	{
		msdf_text_shader = Shader::getShader("text_msdf");
		msdf_text_shader->FindUniformLocations(TEXT_SHADER_UNIFORM_NAMES);
		bmfont_text_shader = Shader::getShader("text_bm");
		bmfont_text_shader->FindUniformLocations(TEXT_SHADER_UNIFORM_NAMES);
	}

	void Update(const InputState& input, double delta_time = (1.0/30.0));

	void Draw(const UniformBufferCamera& camera);

private:
	Shader* msdf_text_shader;
	Shader* bmfont_text_shader;

	Font mops;
	Font fixedsys;
	Text text;
	TextWrap wrapping_text;
	DialogueGui dialoguebox;
};

#endif // HVH_WC_GUI_GUILAYER_H
