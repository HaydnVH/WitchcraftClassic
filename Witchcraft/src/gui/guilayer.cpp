#include "guilayer.h"

GuiLayer::GuiLayer()
:	msdf_text_shader(nullptr),
	bmfont_text_shader(nullptr),
	mops("MopsAntiqua"),
	fixedsys("FixedsysExcelsior"),
	text(fixedsys),
	wrapping_text(mops),
	dialoguebox(mops)
{
//	text.setString(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");
//	text.setSize(1.0f);

//	wrapping_text.setString("In publishing and graphic design, lorem ipsum is a placeholder text used to demonstrate the visual form of a document without relying on meaningful content (also called greeking). Replacing the actual content with placeholder text allows designers to design the form of the content before the content itself has been produced.");
//	wrapping_text.setSize(1.0f);
//	wrapping_text.setDimensions({ 0.5f, 0.2f });
}

void GuiLayer::Update(const InputState& input, double delta_time)
{
//	dialoguebox.Update(input);
}

void GuiLayer::Draw(const UniformBufferCamera& camera)
{
//	bmfont_text_shader->use();
//	bmfont_text_shader->setUniform(UNIFORM_TEXT_GLYPHS_TEXTURE, 0);
//	fixedsys.useTexture(0);

//	bmfont_text_shader->setUniform(UNIFORM_TEXT_POSITION, 0.005f, 0.0f);
//	bmfont_text_shader->setUniform(UNIFORM_TEXT_CLIPRECT, 0.0f, 0.0f, 1.0f, 1.0f);

//	bmfont_text_shader->setUniform(UNIFORM_TEXT_COLOR, 0.0f, 0.0f, 0.0f, 1.0f);
//	bmfont_text_shader->setUniform(UNIFORM_TEXT_OUTLINE, 1.0f);
//	text.Draw(camera);

//	bmfont_text_shader->setUniform(UNIFORM_TEXT_COLOR, 0.95f, 0.9f, 0.8f, 1.0f);
//	bmfont_text_shader->setUniform(UNIFORM_TEXT_OUTLINE, 0.0f);
//	text.Draw(camera);

	/*
	msdf_text_shader->use();
	msdf_text_shader->setUniform(UNIFORM_TEXT_GLYPHS_TEXTURE, 0);
	mops.useTexture(0);

	msdf_text_shader->setUniform(UNIFORM_TEXT_POSITION, 0.005f, 0.0f);
	msdf_text_shader->setUniform(UNIFORM_TEXT_CLIPRECT, 0.0f, 0.0f, 1.0f, 1.0f);

	msdf_text_shader->setUniform(UNIFORM_TEXT_COLOR, vmath::vec4(0, 0, 0, 1));
	msdf_text_shader->setUniform(UNIFORM_TEXT_CUTOFFS, vmath::vec2(0.05f, 0.25f));
	text.Draw(camera);

	msdf_text_shader->setUniform(UNIFORM_TEXT_COLOR, vmath::vec4(0.95f, 0.9f, 0.8f, 1.0f));
	msdf_text_shader->setUniform(UNIFORM_TEXT_CUTOFFS, vmath::vec2(0.5f, 0.75f));
	text.Draw(camera);
	*/
	////

//	msdf_text_shader->use();
//	msdf_text_shader->setUniform(UNIFORM_TEXT_GLYPHS_TEXTURE, 0);
//	mops.useTexture(0);

//	dialoguebox.Draw(camera, msdf_text_shader);
}