#include "dialogue.h"

#include "sys/printlog.h"

void DialogueGui::Update(const InputState& input)
{
	if (show)
	{
		num_glyphs_to_draw += 2;
		int total_glyphs_to_draw = 0;
		for (int i = 0; i < current_line + 3; ++i)
		{
			total_glyphs_to_draw += text.getNumGlyphsOnLine(i);
		}

		if (num_glyphs_to_draw >= total_glyphs_to_draw)
		{
			num_glyphs_to_draw = total_glyphs_to_draw;
			end_of_line = true;

			if (input.controls.buttons & CONTROL_BUTTON_MENUACCEPT)
			{
				if (current_line < (text.getNumLines() - 3))
				{
					current_line += 2;
					if (current_line > (text.getNumLines() - 3))
					{
						current_line = (text.getNumLines() - 3);
					}
					end_of_line = false;
				}
				else
				{
					// Close the dialogue box
					show = false;
				}
			}
		}
	}
	else // show == false
	{
		num_glyphs_to_draw = 0;
	}
}

void DialogueGui::Draw(const UniformBufferCamera& camera, Shader* shader)
{
	if (show == false)
		return;

	float inv_interpolation = 1.0f - camera.frame_interpolation;
	int glyphs_adjustment = 0;
	if (end_of_line == false)
		glyphs_adjustment = (int)round(inv_interpolation * 2.0f);

	// This bit lets the dialogue box have a consistent width relative to the height of the screen,
	// regardless of what the aspect ratio happens to be.
	// Advantage: The dialogue box will always fit the exact same number of glyphs on each line, regardless of aspect ratio.
	// Disadvantage: on narrow vertical screens, the dialogue box can creep beyond the boundries of the screen.
	// How can we resolve this?
	float box_width = 0.8f / camera.aspect_ratio;
	float box_xpos = 0.5f - (box_width * 0.5f);

	// Right now this causes the text system to rebuild the text geometry every single frame.
	// Let's figure out some way to not do that.
	text.setPosition({ box_xpos, 0.7f });
	text.setDimensions({ box_width, 0.3f });

	shader->setUniform(UNIFORM_TEXT_POSITION, text.getPosition() - vmath::vec2(0, current_line * NORMALIZED_VERTICAL_TEXT_FACTOR * 1.5f));
	shader->setUniform(UNIFORM_TEXT_CLIPRECT, box_xpos, 0.6f, box_xpos + box_width, 0.9f);

	shader->setUniform(UNIFORM_TEXT_COLOR, vmath::vec4(0, 0, 0, 1));
	shader->setUniform(UNIFORM_TEXT_CUTOFFS, vmath::vec2(0.05f, 0.25f));

	int total_glyphs_drawn = num_glyphs_to_draw - glyphs_adjustment;

	for (int i = 0; i < current_line+3; ++i)
	{
		if (i < current_line)
		{
			total_glyphs_drawn -= text.getNumGlyphsOnLine(i);
			continue;
		}

		text.DrawLine(camera, i, total_glyphs_drawn);
		if (total_glyphs_drawn > text.getNumGlyphsOnLine(i))
			total_glyphs_drawn -= text.getNumGlyphsOnLine(i);
		else
			break;
	}

	shader->setUniform(UNIFORM_TEXT_COLOR, vmath::vec4(0.7f, 0.6f, 0.4f, 1.0f));
	shader->setUniform(UNIFORM_TEXT_CUTOFFS, vmath::vec2(0.5f, 0.75f));

	total_glyphs_drawn = num_glyphs_to_draw - glyphs_adjustment;
	for (int i = 0; i < current_line+3; ++i)
	{
		if (i < current_line)
		{
			total_glyphs_drawn -= text.getNumGlyphsOnLine(i);
			continue;
		}

		text.DrawLine(camera, i, total_glyphs_drawn);
		if (total_glyphs_drawn > text.getNumGlyphsOnLine(i))
			total_glyphs_drawn -= text.getNumGlyphsOnLine(i);
		else
			break;
	}
}