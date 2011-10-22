// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "UI/Core/CommitDetails.hpp"
#include "UI/Core/TreeThread.hpp"

#include "UI/Graphics/ConfirmCommitDialog.hpp"
#include "UI/Graphics/GraphicalValue.hpp"

#include "UI/Graphics/OptionLayout.hpp"

using namespace cf3::common;
using namespace cf3::UI::Core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

OptionLayout::OptionLayout(QWidget * parent)
  : QFormLayout(parent)
{

}

//////////////////////////////////////////////////////////////////////////

OptionLayout::~OptionLayout()
{
  this->clear_options();
}

//////////////////////////////////////////////////////////////////////////

void OptionLayout::options(QMap<QString, QString> & options, bool all) const
{
  QMap<QString, GraphicalValue *>::const_iterator it = m_options.begin();

  for( ; it != m_options.end() ; it++)
  {
    GraphicalValue * value = it.value();

    if(all || value->is_modified())
      options[ it.key() ] = value->value_string();
  }
}

//////////////////////////////////////////////////////////////////////////

void OptionLayout::commit_options()
{
  QMap<QString, GraphicalValue *>::const_iterator it = m_options.begin();

  for( ; it != m_options.end() ; it++)
    it.value()->commit();
}

//////////////////////////////////////////////////////////////////////////

void OptionLayout::clear_options()
{
  QMap<QString, GraphicalValue *>::const_iterator it = m_options.begin();

  while(it != m_options.end())
  {
    delete labelForField(it.value()); // delete the associated label
    delete it.value();
    it++;
  }

  m_options.clear();
}

//////////////////////////////////////////////////////////////////////////

bool OptionLayout::is_modified() const
{
  bool modified = false;

  QMap<QString, GraphicalValue *>::const_iterator it = m_options.begin();

  for( ; it != m_options.end() && !modified ; it++)
    modified = it.value()->is_modified();

  return modified;
}

//////////////////////////////////////////////////////////////////////////

void OptionLayout::modified_options(CommitDetails & commitDetails) const
{
  QMap<QString, GraphicalValue *>::const_iterator it = m_options.begin();

  while(it != m_options.end())
  {
    GraphicalValue * value = it.value();

    if(value->is_modified())
    {
      QString oldValue = value->original_value_string();
      QString newValue = value->value_string();

      commitDetails.set_option(it.key(), oldValue, newValue);
    }

    it++;
  }
}

//////////////////////////////////////////////////////////////////////////

void OptionLayout::add_option(cf3::common::Option::ConstPtr option)
{
  GraphicalValue * value = GraphicalValue::create_from_option(option);
  QString name(option->name().c_str());

  m_options[name] = value;

  if( !option->pretty_name().empty() )
    name = option->pretty_name().c_str();

  value->setToolTip(option->description().c_str());

  addRow(name + ':', value);

  // forward the signal
  connect(value, SIGNAL(value_changed()), this, SIGNAL(value_changed()));
}

//////////////////////////////////////////////////////////////////////////

bool OptionLayout::has_options() const
{
  return !m_options.isEmpty();
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
