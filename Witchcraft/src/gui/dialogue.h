#ifndef HVH_WC_GUI_DIALOGUE_H
#define HVH_WC_GUI_DIALOGUE_H

#include "text_wrap.h"
#include "graphics/shader.h"
#include "sys/input.h"

class DialogueGui
{
public:
	DialogueGui(Font& font)
	:	text(font),
		current_line(0),
		num_glyphs_to_draw(0),
		show(true),
		end_of_line(false)
	{
		text.setPosition({ 0.25f, 0.7f });
		text.setDimensions({0.5f, 0.2f});
		text.setString("In publishing and graphic design, lorem ipsum is a placeholder text used to demonstrate the visual form of a document without relying on meaningful content (also called greeking). Replacing the actual content with placeholder text allows designers to design the form of the content before the content itself has been produced.");
		text.setSize(1.0f);

		// This is just so we have some lines to work with before the first draw.
		text.BuildLines(0.75f);
	}

	void Update(const InputState& input);
	void Draw(const UniformBufferCamera& camera, Shader* shader);

	void ShowDialogue(std::string str)
	{
		text.setString(str);
		current_line = 0;
		num_glyphs_to_draw = 0;
		show = true;
		end_of_line = false;
	}

private:
	TextWrap text;
	int current_line;
	int num_glyphs_to_draw;

	bool show;
	bool end_of_line;
};

#endif // HVH_WC_GUI_DIALOGUE_H