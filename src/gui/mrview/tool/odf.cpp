/*
   Copyright 2009 Brain Research Institute, Melbourne, Australia

   Written by J-Donald Tournier, 13/11/09.

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

#include <QLabel>
#include <QSplitter>
#include <QListView>
#include <QCheckBox>
#include <QComboBox>

#include "mrtrix.h"
#include "gui/dialog/file.h"
#include "gui/dwi/render_frame.h"
#include "gui/mrview/window.h"
#include "gui/mrview/tool/odf.h"
#include "gui/mrview/tool/list_model_base.h"
#include "gui/mrview/mode/base.h"

namespace MR
{
  namespace GUI
  {
    namespace MRView
    {
      namespace Tool
      {


        class ODF::Model : public ListModelBase
        {
          public:
            Model (QObject* parent) : 
              ListModelBase (parent) { }

            QVariant data (const QModelIndex& index, int role) const {
              if (!index.isValid()) return QVariant();
              if (role != Qt::DisplayRole) return QVariant();
              return shorten (items[index.row()]->get_filename(), 35, 0).c_str();
            }

            bool setData (const QModelIndex& index, const QVariant& value, int role) {
              return QAbstractItemModel::setData (index, value, role);
            }

            Qt::ItemFlags flags (const QModelIndex& index) const {
              if (!index.isValid()) return 0;
              return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
            }

            void add_items (const std::vector<std::string>& list) {
              beginInsertRows (QModelIndex(), items.size(), items.size()+list.size());
              for (size_t i = 0; i < list.size(); ++i) {
                try {
                  MR::Image::Header header (list[i]);
                  items.push_back (new Image (header));
                }
                catch (Exception& E) {
                  E.display();
                }
              }
              endInsertRows();
            }

            Image* get_image (QModelIndex& index) {
              return index.isValid() ? dynamic_cast<Image*>(items[index.row()]) : NULL;
            }
        };




        ODF::ODF (Window& main_window, Dock* parent) :
          Base (main_window, parent) { 
            QVBoxLayout *main_box = new QVBoxLayout (this);
            main_box->setContentsMargins (0, 0, 0, 0);
            main_box->setSpacing (0);

            QSplitter* splitter = new QSplitter (Qt::Vertical, parent);
            main_box->addWidget (splitter);

            render_frame = new DWI::RenderFrame (this);
            splitter->addWidget (render_frame);



            QFrame* frame = new QFrame (this);
            frame->setFrameStyle (QFrame::NoFrame);
            splitter->addWidget (frame);

            main_box = new QVBoxLayout (frame);

            QHBoxLayout* layout = new QHBoxLayout;
            layout->setContentsMargins (0, 0, 0, 0);
            layout->setSpacing (0);

            QPushButton* button = new QPushButton (this);
            button->setToolTip (tr ("Open Image"));
            button->setIcon (QIcon (":/open.svg"));
            connect (button, SIGNAL (clicked()), this, SLOT (image_open_slot ()));
            layout->addWidget (button, 1);

            button = new QPushButton (this);
            button->setToolTip (tr ("Close Image"));
            button->setIcon (QIcon (":/close.svg"));
            connect (button, SIGNAL (clicked()), this, SLOT (image_close_slot ()));
            layout->addWidget (button, 1);

            main_box->addLayout (layout, 0);

            image_list_view = new QListView (this);
            image_list_view->setSelectionMode (QAbstractItemView::SingleSelection);
            image_list_view->setDragEnabled (true);
            image_list_view->viewport()->setAcceptDrops (true);
            image_list_view->setDropIndicatorShown (true);

            image_list_model = new Model (this);
            image_list_view->setModel (image_list_model);

            main_box->addWidget (image_list_view, 1);


            QGroupBox* group_box = new QGroupBox (tr("Display settings"));
            main_box->addWidget (group_box);
            QGridLayout* box_layout = new QGridLayout;
            box_layout->setContentsMargins (0, 0, 0, 0);
            box_layout->setSpacing (0);
            group_box->setLayout (box_layout);

            lock_orientation_to_image_box = new QCheckBox ("auto align");
            lock_orientation_to_image_box->setChecked (true);
            connect (lock_orientation_to_image_box, SIGNAL (stateChanged(int)), this, SLOT (lock_orientation_to_image_slot(int)));
            box_layout->addWidget (lock_orientation_to_image_box, 0, 0, 1, 2);

            interpolation_box = new QCheckBox ("interpolation");
            interpolation_box->setChecked (true);
            connect (interpolation_box, SIGNAL (stateChanged(int)), this, SLOT (interpolation_slot(int)));
            box_layout->addWidget (interpolation_box, 1, 0, 1, 2);

            show_axes_box = new QCheckBox ("show axes");
            show_axes_box->setChecked (true);
            connect (show_axes_box, SIGNAL (stateChanged(int)), this, SLOT (show_axes_slot(int)));
            box_layout->addWidget (show_axes_box, 2, 0, 1, 2);

            colour_by_direction_box = new QCheckBox ("colour by direction");
            colour_by_direction_box->setChecked (true);
            connect (colour_by_direction_box, SIGNAL (stateChanged(int)), this, SLOT (colour_by_direction_slot(int)));
            box_layout->addWidget (colour_by_direction_box, 3, 0, 1, 2);

            use_lighting_box = new QCheckBox ("use lighting");
            use_lighting_box->setChecked (true);
            connect (use_lighting_box, SIGNAL (stateChanged(int)), this, SLOT (use_lighting_slot(int)));
            box_layout->addWidget (use_lighting_box, 4, 0, 1, 2);

            hide_negative_lobes_box = new QCheckBox ("hide negative lobes");
            hide_negative_lobes_box->setChecked (true);
            connect (hide_negative_lobes_box, SIGNAL (stateChanged(int)), this, SLOT (hide_negative_lobes_slot(int)));
            box_layout->addWidget (hide_negative_lobes_box, 5, 0, 1, 2);


            box_layout->addWidget (new QLabel ("lmax"), 6, 0);
            lmax_selector = new QSpinBox (this);
            lmax_selector->setMinimum (2);
            lmax_selector->setMaximum (16);
            lmax_selector->setSingleStep (2);
            lmax_selector->setValue (8);
            connect (lmax_selector, SIGNAL (valueChanged(int)), this, SLOT(lmax_slot(int)));
            box_layout->addWidget (lmax_selector, 6, 1);

            box_layout->addWidget (new QLabel ("detail"), 7, 0);
            level_of_detail_selector = new QSpinBox (this);
            level_of_detail_selector->setMinimum (1);
            level_of_detail_selector->setMaximum (7);
            level_of_detail_selector->setSingleStep (1);
            level_of_detail_selector->setValue (3);
            connect (level_of_detail_selector, SIGNAL (valueChanged(int)), this, SLOT(level_of_detail_slot(int)));
            box_layout->addWidget (level_of_detail_selector, 7, 1);


            overlay_frame = new QGroupBox (tr("Overlay"));
            main_box->addWidget (overlay_frame);
            box_layout = new QGridLayout;
            overlay_frame->setLayout (box_layout);

/*
            box_layout->addWidget (new QLabel ("min"), 1, 0);
            min_value = new AdjustButton (this, 0.1);
            connect (min_value, SIGNAL (valueChanged()), this, SLOT (values_changed()));
            box_layout->addWidget (min_value, 1, 1);

            group_box = new QGroupBox (tr("Thresholds"));
            main_box->addWidget (group_box);
            box_layout = new QGridLayout;
            group_box->setLayout (box_layout);

            threshold_upper_box = new QCheckBox ("max");
            connect (threshold_upper_box, SIGNAL (stateChanged(int)), this, SLOT (threshold_upper_changed(int)));
            box_layout->addWidget (threshold_upper_box, 0, 0);
            threshold_upper = new AdjustButton (this, 0.1);
            connect (threshold_upper, SIGNAL (valueChanged()), this, SLOT (threshold_upper_value_changed()));
            box_layout->addWidget (threshold_upper, 0, 1);

            threshold_lower_box = new QCheckBox ("min");
            connect (threshold_lower_box, SIGNAL (stateChanged(int)), this, SLOT (threshold_lower_changed(int)));
            box_layout->addWidget (threshold_lower_box, 1, 0);
            threshold_lower = new AdjustButton (this, 0.1);
            connect (threshold_lower, SIGNAL (valueChanged()), this, SLOT (threshold_lower_value_changed()));
            box_layout->addWidget (threshold_lower, 1, 1);

            opacity = new QSlider (Qt::Horizontal);
            opacity->setRange (1,1000);
            opacity->setSliderPosition (int (1000));
            connect (opacity, SIGNAL (valueChanged (int)), this, SLOT (update_slot (int)));
            main_box->addWidget (new QLabel ("opacity"), 0);
            main_box->addWidget (opacity, 0);

            connect (image_list_view->selectionModel(),
                SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                SLOT (selection_changed_slot(const QItemSelection &, const QItemSelection &)) );

            connect (image_list_view, SIGNAL (clicked (const QModelIndex&)), this, SLOT (toggle_shown_slot (const QModelIndex&)));

            update_selection();
            */

            splitter->setStretchFactor (0, 1);
            splitter->setStretchFactor (1, 0);

            hide_negative_lobes_slot (0);
            show_axes_slot (0);
            colour_by_direction_slot (0);
            use_lighting_slot (0);
            lmax_slot (0);
            level_of_detail_slot (0);
            lock_orientation_to_image_slot (0);
          }


        void ODF::draw2D (const Projection& projection) 
        {
          lock_orientation_to_image_slot(0);

          if (overlay_frame->isChecked())
            TRACE;

          /*
          float overlay_opacity = opacity->value() / 1.0e3f;

          // set up OpenGL environment:
          glEnable (GL_BLEND);
          glEnable (GL_TEXTURE_3D);
          glDisable (GL_DEPTH_TEST);
          glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
          glDepthMask (GL_FALSE);
          glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
          glBlendFunc (GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
          glBlendEquation (GL_FUNC_ADD);
          glBlendColor (1.0f, 1.0f, 1.0f, overlay_opacity);

          bool need_to_update = false;
          for (int i = 0; i < image_list_model->rowCount(); ++i) {
            if (image_list_model->items[i]->show) {
              Image* image = dynamic_cast<Image*>(image_list_model->items[i]);
              need_to_update |= !finite (image->intensity_min());
              image->render3D (projection, projection.depth_of (window.focus()));
            }
          }

          if (need_to_update)
            update_selection();

          DEBUG_OPENGL;

          glDisable (GL_TEXTURE_3D);
          */
        }




        void ODF::draw3D (const Projection& transform)
        { 
        }


        void ODF::showEvent (QShowEvent* event) 
        {
          connect (&window, SIGNAL (focusChanged()), this, SLOT (onFocusChanged()));
          onFocusChanged();
        }


        void ODF::closeEvent (QCloseEvent* event) { window.disconnect (this); }

        void ODF::onFocusChanged () 
        {
          QModelIndex index = image_list_view->selectionModel()->currentIndex();
          Image* image = image_list_model->get_image (index);
          if (!image)
            return;

          Math::Vector<float> values (image->interp.dim(3));
          image->interp.scanner (window.focus());
          for (image->interp[3] = 0; image->interp[3] < image->interp.dim(3); ++image->interp[3])
            values[image->interp[3]] = image->interp.value().real(); 

          render_frame->set (values);

        }



        void ODF::image_open_slot ()
        {
          std::vector<std::string> list = Dialog::File::get_images (this, "Select overlay images to open");
          if (list.empty())
            return;

          size_t previous_size = image_list_model->rowCount();
          image_list_model->add_items (list);
          QModelIndex first = image_list_model->index (previous_size, 0, QModelIndex());
          image_list_view->selectionModel()->select (QItemSelection (first, first), QItemSelectionModel::Select);
        }



        void ODF::image_close_slot ()
        {
          QModelIndexList indexes = image_list_view->selectionModel()->selectedIndexes();
          while (indexes.size()) {
            image_list_model->remove_item (indexes.first());
            indexes = image_list_view->selectionModel()->selectedIndexes();
          }
        }



        void ODF::lock_orientation_to_image_slot (int unused) {
          if (lock_orientation_to_image_box->isChecked()) {
            render_frame->makeCurrent();
            render_frame->set_rotation (window.get_current_mode()->projection.modelview());
            window.makeGLcurrent();
          }
        }

        void ODF::colour_by_direction_slot (int unused) { render_frame->set_color_by_dir (colour_by_direction_box->isChecked()); }
        void ODF::hide_negative_lobes_slot (int unused) { render_frame->set_hide_neg_lobes (hide_negative_lobes_box->isChecked()); }
        void ODF::use_lighting_slot (int unused) { render_frame->set_use_lighting (use_lighting_box->isChecked()); }
        void ODF::interpolation_slot (int unused) { TRACE; }
        void ODF::show_axes_slot (int unused) { render_frame->set_show_axes (show_axes_box->isChecked()); }
        void ODF::level_of_detail_slot (int value) { render_frame->set_LOD (level_of_detail_selector->value()); }
        void ODF::lmax_slot (int value) { render_frame->set_lmax (lmax_selector->value()); }

        void ODF::update_slot (int unused) {
          window.updateGL();
        }
/*
        void ODF::colourmap_changed (int index) 
        {
          QModelIndexList indices = image_list_view->selectionModel()->selectedIndexes();
          for (int i = 0; i < indices.size(); ++i) {
            Image* overlay = dynamic_cast<Image*> (image_list_model->get_image (indices[i]));
            overlay->set_colourmap (index);
          }
          window.updateGL();
        }


        void ODF::values_changed ()
        {
          QModelIndexList indices = image_list_view->selectionModel()->selectedIndexes();
          for (int i = 0; i < indices.size(); ++i) {
            Image* overlay = dynamic_cast<Image*> (image_list_model->get_image (indices[i]));
            overlay->set_windowing (min_value->value(), max_value->value());
          }
          window.updateGL();
        }


        void ODF::threshold_lower_changed (int unused) 
        {
          QModelIndexList indices = image_list_view->selectionModel()->selectedIndexes();
          for (int i = 0; i < indices.size(); ++i) {
            Image* overlay = dynamic_cast<Image*> (image_list_model->get_image (indices[i]));
            overlay->lessthan = threshold_lower->value();
            overlay->set_use_discard_lower (threshold_lower_box->isChecked());
          }
          threshold_lower->setEnabled (indices.size() && threshold_lower_box->isChecked());
          window.updateGL();
        }


        void ODF::threshold_upper_changed (int unused)
        {
          QModelIndexList indices = image_list_view->selectionModel()->selectedIndexes();
          for (int i = 0; i < indices.size(); ++i) {
            Image* overlay = dynamic_cast<Image*> (image_list_model->get_image (indices[i]));
            overlay->greaterthan = threshold_upper->value();
            overlay->set_use_discard_upper (threshold_upper_box->isChecked());
          }
          threshold_upper->setEnabled (indices.size() && threshold_upper_box->isChecked());
          window.updateGL();
        }



        void ODF::threshold_lower_value_changed ()
        {
          if (threshold_lower_box->isChecked()) {
            QModelIndexList indices = image_list_view->selectionModel()->selectedIndexes();
            for (int i = 0; i < indices.size(); ++i) {
              Image* overlay = dynamic_cast<Image*> (image_list_model->get_image (indices[i]));
              overlay->lessthan = threshold_lower->value();
            }
          }
          window.updateGL();
        }



        void ODF::threshold_upper_value_changed ()
        {
          if (threshold_upper_box->isChecked()) {
            QModelIndexList indices = image_list_view->selectionModel()->selectedIndexes();
            for (int i = 0; i < indices.size(); ++i) {
              Image* overlay = dynamic_cast<Image*> (image_list_model->get_image (indices[i]));
              overlay->greaterthan = threshold_upper->value();
            }
          }
          window.updateGL();
        }


*/
        void ODF::selection_changed_slot (const QItemSelection &, const QItemSelection &)
        {
          update_selection();
        }


        void ODF::update_selection () 
        {
          /*
          QModelIndexList indices = image_list_view->selectionModel()->selectedIndexes();
          colourmap_combobox->setEnabled (indices.size());
          max_value->setEnabled (indices.size());
          min_value->setEnabled (indices.size());
          threshold_lower_box->setEnabled (indices.size());
          threshold_upper_box->setEnabled (indices.size());
          threshold_lower->setEnabled (indices.size());
          threshold_upper->setEnabled (indices.size());

          if (!indices.size())
            return;

          float rate = 0.0f, min_val = 0.0f, max_val = 0.0f;
          float threshold_lower_val = 0.0f, threshold_upper_val = 0.0f;
          int num_threshold_lower = 0, num_threshold_upper = 0;
          int colourmap_index = -2;
          for (int i = 0; i < indices.size(); ++i) {
            Image* overlay = dynamic_cast<Image*> (image_list_model->get_image (indices[i]));
            if (colourmap_index != int(overlay->colourmap())) {
              if (colourmap_index == -2)
                colourmap_index = overlay->colourmap();
              else 
                colourmap_index = -1;
            }
            rate += overlay->scaling_rate();
            min_val += overlay->scaling_min();
            max_val += overlay->scaling_max();
            num_threshold_lower += overlay->use_discard_lower();
            num_threshold_upper += overlay->use_discard_upper();
            if (!finite (overlay->lessthan)) 
              overlay->lessthan = overlay->intensity_min();
            if (!finite (overlay->greaterthan)) 
              overlay->greaterthan = overlay->intensity_max();
            threshold_lower_val += overlay->lessthan;
            threshold_upper_val += overlay->greaterthan;
          }
          rate /= indices.size();
          min_val /= indices.size();
          max_val /= indices.size();
          threshold_lower_val /= indices.size();
          threshold_upper_val /= indices.size();

          colourmap_combobox->setCurrentIndex (colourmap_index);

          min_value->setRate (rate);
          max_value->setRate (rate);
          min_value->setValue (min_val);
          max_value->setValue (max_val);

          threshold_lower_box->setCheckState (num_threshold_lower ? 
              ( num_threshold_lower == indices.size() ? 
                Qt::Checked :
                Qt::PartiallyChecked ) : 
              Qt::Unchecked);
          threshold_lower->setValue (threshold_lower_val);
          threshold_lower->setRate (rate);

          threshold_upper_box->setCheckState (num_threshold_upper ? 
              ( num_threshold_upper == indices.size() ? 
                Qt::Checked :
                Qt::PartiallyChecked ) : 
              Qt::Unchecked);
          threshold_upper->setValue (threshold_upper_val);
          threshold_upper->setRate (rate);*/
        }


      }
    }
  }
}





