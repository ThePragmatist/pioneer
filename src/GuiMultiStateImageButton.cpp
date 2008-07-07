#include "libs.h"
#include "Gui.h"

namespace Gui {
MultiStateImageButton::MultiStateImageButton(): Button()
{
	m_curState = 0;
	m_isSelected = true;
	Button::onClick.connect(sigc::mem_fun(this, &MultiStateImageButton::OnActivate));
}

MultiStateImageButton::~MultiStateImageButton()
{
	for (std::vector<State>::iterator i = m_states.begin(); i != m_states.end(); ++i) {
		delete (*i).image;
	}
}

void MultiStateImageButton::StateNext()
{
	m_curState++;
	if (m_curState >= (signed)m_states.size()) m_curState = 0;
}

void MultiStateImageButton::StatePrev()
{
	m_curState--;
	if (m_curState < 0) m_curState = (signed)m_states.size()-1;
}

void MultiStateImageButton::OnActivate()
{
	// only iterate through states once widget is selected.
	if (m_isSelected) StateNext();
	else {
		m_isSelected = true;
		onSelect.emit(this);
	}
	onClick.emit(this);
}

void MultiStateImageButton::SetSelected(bool state)
{
	m_isSelected = state;
}

void MultiStateImageButton::GetSizeRequested(float size[2])
{
	assert(m_states.size());
	m_states[0].image->GetSizeRequested(size);
}

void MultiStateImageButton::Draw()
{
	m_states[m_curState].image->Draw();
}

void MultiStateImageButton::AddState(int state, const char *filename)
{
	State s;
	s.state = state;
	s.image = new Image(filename);
	m_states.push_back(s);
	float size[2];
	s.image->GetSizeRequested(size);
	SetSize(size[0], size[1]);
}

}