/*
    Copyright 2013 Brain Research Institute, Melbourne, Australia

    Written by David Raffelt, 01/02/13.

    This file is part of MRtrix.

    MRtrix is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MRtrix is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MRtrix.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <QGroupBox>
#include <QGridLayout>

#include "gui/mrview/tool/tractography/scalar_file_options.h"
#include "math/vector.h"
#include "gui/dialog/lighting.h"
#include "gui/mrview/colourmap.h"

namespace MR
{
  namespace GUI
  {
    namespace MRView
    {
      namespace Tool
      {

        ScalarFileOptions::ScalarFileOptions (Window& main_window, Dock* parent) :
          Base (main_window, parent)
        {
          main_box = new QVBoxLayout (this);
          main_box->setContentsMargins (5, 5, 5, 5);
          main_box->setSpacing (5);

          QHBoxLayout* hlayout = new QHBoxLayout;
          hlayout->setContentsMargins (0, 0, 0, 0);
          hlayout->setSpacing (0);

          file_button = new QPushButton (this);
          file_button->setToolTip (tr ("Open scalar track file"));
          connect (file_button, SIGNAL (clicked()), this, SLOT (open_track_scalar_file_slot ()));
          hlayout->addWidget (file_button);


          // Colourmap menu:
          colourmap_menu = new QMenu (tr ("Colourmap menu"), this);

          ColourMap::create_menu (this, colourmap_group, colourmap_menu, colourmap_actions, false, false);
          connect (colourmap_group, SIGNAL (triggered (QAction*)), this, SLOT (select_colourmap_slot()));
          colourmap_actions[1]->setChecked (true);

          colourmap_menu->addSeparator();

          show_colour_bar = colourmap_menu->addAction (tr ("Show colour bar"), this, SLOT (show_colour_bar_slot()));
          show_colour_bar->setCheckable (true);
          show_colour_bar->setChecked (true);
          addAction (show_colour_bar);

          invert_action = colourmap_menu->addAction (tr ("Invert"), this, SLOT (invert_colourmap_slot()));
          invert_action->setCheckable (true);
          addAction (invert_action);

          scalarfile_by_direction = colourmap_menu->addAction (tr ("Colour by direction"), this, SLOT (scalarfile_by_direction_slot()));
          scalarfile_by_direction->setCheckable (true);
          addAction (scalarfile_by_direction);

          colourmap_menu->addSeparator();

          QAction* reset_intensity = colourmap_menu->addAction (tr ("Reset intensity"), this, SLOT (reset_intensity_slot()));
          addAction (reset_intensity);

          colourmap_button = new QToolButton (this);
          colourmap_button->setToolTip (tr ("Colourmap menu"));
          colourmap_button->setIcon (QIcon (":/colourmap.svg"));
          colourmap_button->setPopupMode (QToolButton::InstantPopup);
          colourmap_button->setMenu (colourmap_menu);
          hlayout->addWidget (colourmap_button);

          main_box->addLayout (hlayout);

          QGroupBox* group_box = new QGroupBox ("Intensity range");
          QGridLayout* layout = new QGridLayout;
          main_box->addWidget (group_box);
          group_box->setLayout (layout);

          layout->addWidget (new QLabel ("min"), 0, 0);
          min_entry = new AdjustButton (this);
          connect (min_entry, SIGNAL (valueChanged()), this, SLOT (on_set_scaling_slot()));
          layout->addWidget (min_entry, 0, 1);

          layout->addWidget (new QLabel ("max"), 1, 0);
          max_entry = new AdjustButton (this);
          connect (max_entry, SIGNAL (valueChanged()), this, SLOT (on_set_scaling_slot()));
          layout->addWidget (max_entry, 1, 1);

          group_box = new QGroupBox (tr("Thresholds"));
          main_box->addWidget (group_box);
          layout = new QGridLayout;
          group_box->setLayout (layout);

          threshold_upper_box = new QCheckBox ("max");
          connect (threshold_upper_box, SIGNAL (stateChanged(int)), this, SLOT (threshold_upper_changed(int)));
          layout->addWidget (threshold_upper_box, 0, 0);
          threshold_upper = new AdjustButton (this, 0.1);
          connect (threshold_upper, SIGNAL (valueChanged()), this, SLOT (threshold_upper_value_changed()));
          layout->addWidget (threshold_upper, 0, 1);

          threshold_lower_box = new QCheckBox ("min");
          connect (threshold_lower_box, SIGNAL (stateChanged(int)), this, SLOT (threshold_lower_changed(int)));
          layout->addWidget (threshold_lower_box, 1, 0);
          threshold_lower = new AdjustButton (this, 0.1);
          connect (threshold_lower, SIGNAL (valueChanged()), this, SLOT (threshold_lower_value_changed()));
          layout->addWidget (threshold_lower, 1, 1);


          main_box->addStretch ();
          setMinimumSize (main_box->minimumSize());
        }


        void ScalarFileOptions::set_tractogram (Tractogram* selected_tractogram) {
          tractogram = selected_tractogram;
          update_tool_display();
        }


        void ScalarFileOptions::clear_tool_display () {
          file_button->setText ("");
          file_button->setEnabled (false);
          min_entry->setEnabled (false);
          max_entry->setEnabled (false);
          min_entry->clear();
          max_entry->clear();
          threshold_lower_box->setChecked(false);
          threshold_upper_box->setChecked(false);
          threshold_lower_box->setEnabled (false);
          threshold_upper_box->setEnabled (false);
          threshold_lower->setEnabled (false);
          threshold_upper->setEnabled (false);
          threshold_lower->clear();
          threshold_upper->clear();
          colourmap_menu->setEnabled (false);
        }


        void ScalarFileOptions::update_tool_display () {

          if (!tractogram) {
            clear_tool_display ();
            return;
          }

          if (tractogram->scalar_filename.size()) {
            file_button->setEnabled (true);
            min_entry->setEnabled (true);
            max_entry->setEnabled (true);
            min_entry->setRate (tractogram->scaling_rate());
            max_entry->setRate (tractogram->scaling_rate());

            threshold_lower_box->setEnabled (true);
            if (tractogram->use_discard_lower()) {
              threshold_lower_box->setChecked(true);
              threshold_lower->setEnabled (true);
            } else {
              threshold_lower->setEnabled (false);
              threshold_lower_box->setChecked(false);
            }
            threshold_upper_box->setEnabled (true);
            if (tractogram->use_discard_upper()) {
              threshold_upper_box->setChecked (true);
              threshold_upper->setEnabled (true);
            } else {
              threshold_upper->setEnabled (false);
              threshold_upper_box->setChecked (false);
            }
            threshold_lower->setRate (tractogram->scaling_rate());
            threshold_lower->setValue (tractogram->lessthan);
            threshold_upper->setRate (tractogram->scaling_rate());
            threshold_upper->setValue (tractogram->greaterthan);
            colourmap_menu->setEnabled (true);
            colourmap_actions[tractogram->colourmap()]->setChecked (true);
            if (tractogram->scalar_filename.length()) {
              file_button->setText (shorten (tractogram->scalar_filename, 35, 0).c_str());
              min_entry->setValue (tractogram->scaling_min());
              max_entry->setValue (tractogram->scaling_max());
            } else {
              file_button->setText ("");
            }
          } else {
            clear_tool_display ();
            file_button->setText (tr("Open File"));
            file_button->setEnabled (true);
          }
        }


        bool ScalarFileOptions::open_track_scalar_file_slot ()
        {
          std::string scalar_file = Dialog::File::get_file (this, "Select track scalar to open", "Track Scalar files (*.tsf)");
          if (scalar_file.empty())
            return false;

          try {
            tractogram->load_track_scalars (scalar_file);
            tractogram->color_type = ScalarFile;
            tractogram->recompile();
            set_tractogram (tractogram);
          } 
          catch (Exception& E) {
            E.display();
            return false;
          }
          return true;
        }


        void ScalarFileOptions::show_colour_bar_slot ()
        {
          if (tractogram) {
            tractogram->show_colour_bar = show_colour_bar->isChecked();
            window.updateGL();
          }
        }


        void ScalarFileOptions::select_colourmap_slot ()
        {
          if (tractogram) {
            QAction* action = colourmap_group->checkedAction();
            size_t n = 0;
            while (action != colourmap_actions[n])
              ++n;
            tractogram->set_colourmap (n);
            window.updateGL();
          }
        }


        void ScalarFileOptions::on_set_scaling_slot ()
        {
          if (tractogram) {
            tractogram->set_windowing (min_entry->value(), max_entry->value());
            window.updateGL();
          }
        }


        void ScalarFileOptions::threshold_lower_changed (int unused)
        {
          if (tractogram) {
            threshold_lower->setEnabled (threshold_lower_box->isChecked());
            tractogram->set_use_discard_lower (threshold_lower_box->isChecked());
            tractogram->recompile();
            window.updateGL();
          }
        }


        void ScalarFileOptions::threshold_upper_changed (int unused)
        {
          if (tractogram) {
            threshold_upper->setEnabled (threshold_upper_box->isChecked());
            tractogram->set_use_discard_upper (threshold_upper_box->isChecked());
            tractogram->recompile();
            window.updateGL();
          }
        }



        void ScalarFileOptions::threshold_lower_value_changed ()
        {
          if (tractogram && threshold_lower_box->isChecked()) {
            tractogram->lessthan = threshold_lower->value();
            window.updateGL();
          }
        }



        void ScalarFileOptions::threshold_upper_value_changed ()
        {
          if (tractogram && threshold_upper_box->isChecked()) {
            tractogram->greaterthan = threshold_upper->value();
            window.updateGL();
          }
        }


        void ScalarFileOptions::scalarfile_by_direction_slot ()
        {
          if (tractogram) {
            tractogram->scalarfile_by_direction = scalarfile_by_direction->isChecked();
            tractogram->recompile();
            window.updateGL();
          }
        }

        void ScalarFileOptions::reset_intensity_slot ()
        {
          if (tractogram) {
            tractogram->reset_windowing();
            update_tool_display ();
            window.updateGL();
          }
        }


        void ScalarFileOptions::invert_colourmap_slot ()
        {
          if (tractogram) {
            tractogram->set_invert_scale (invert_action->isChecked());
            tractogram->recompile();
            window.updateGL();
          }
        }

      }
    }
  }
}
