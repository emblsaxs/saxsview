project (QWT CXX)

find_package(Qt5 5.12.0 COMPONENTS Widgets Concurrent PrintSupport Svg OpenGL REQUIRED)

set (QWT_SOURCES
     qwt_abstract_legend.cpp
     qwt_abstract_scale.cpp
     qwt_abstract_scale_draw.cpp
     qwt_abstract_slider.cpp
     qwt_analog_clock.cpp
     qwt_arrow_button.cpp
     qwt_clipper.cpp
     qwt_color_map.cpp
     qwt_column_symbol.cpp
     qwt_compass.cpp
     qwt_compass_rose.cpp
     qwt_counter.cpp
     qwt_curve_fitter.cpp
     qwt_date.cpp
     qwt_date_scale_draw.cpp
     qwt_date_scale_engine.cpp
     qwt_dial.cpp
     qwt_dial_needle.cpp
     qwt_dyngrid_layout.cpp
     qwt_event_pattern.cpp
     qwt_graphic.cpp
     qwt_interval.cpp
     qwt_interval_symbol.cpp
     qwt_knob.cpp
     qwt_legend.cpp
     qwt_legend_data.cpp
     qwt_legend_label.cpp
     qwt_magnifier.cpp
     qwt_math.cpp
     qwt_matrix_raster_data.cpp
     qwt_null_paintdevice.cpp
     qwt_painter_command.cpp
     qwt_painter.cpp
     qwt_panner.cpp
     qwt_picker.cpp
     qwt_picker_machine.cpp
     qwt_pixel_matrix.cpp
     qwt_plot_abstract_barchart.cpp
     qwt_plot_axis.cpp
     qwt_plot_barchart.cpp
     qwt_plot_canvas.cpp
     qwt_plot.cpp
     qwt_plot_curve.cpp
     qwt_plot_dict.cpp
     qwt_plot_directpainter.cpp
     qwt_plot_glcanvas.cpp
     qwt_plot_grid.cpp
     qwt_plot_histogram.cpp
     qwt_plot_intervalcurve.cpp
     qwt_plot_item.cpp
     qwt_plot_layout.cpp
     qwt_plot_legenditem.cpp
     qwt_plot_magnifier.cpp
     qwt_plot_marker.cpp
     qwt_plot_multi_barchart.cpp
     qwt_plot_panner.cpp
     qwt_plot_picker.cpp
     qwt_plot_rasteritem.cpp
     qwt_plot_renderer.cpp
     qwt_plot_rescaler.cpp
     qwt_plot_scaleitem.cpp
     qwt_plot_seriesitem.cpp
     qwt_plot_shapeitem.cpp
     qwt_plot_spectrocurve.cpp
     qwt_plot_spectrogram.cpp
     qwt_plot_svgitem.cpp
     qwt_plot_textlabel.cpp
     qwt_plot_tradingcurve.cpp
     qwt_plot_xml.cpp
     qwt_plot_zoneitem.cpp
     qwt_plot_zoomer.cpp
     qwt_point_3d.cpp
     qwt_point_data.cpp
     qwt_point_mapper.cpp
     qwt_point_polar.cpp
     qwt_raster_data.cpp
     qwt_round_scale_draw.cpp
     qwt_sampling_thread.cpp
     qwt_scale_div.cpp
     qwt_scale_draw.cpp
     qwt_scale_engine.cpp
     qwt_scale_map.cpp
     qwt_scale_widget.cpp
     qwt_series_data.cpp
     qwt_slider.cpp
     qwt_spline.cpp
     qwt_symbol.cpp
     qwt_system_clock.cpp
     qwt_text.cpp
     qwt_text_engine.cpp
     qwt_text_label.cpp
     qwt_thermo.cpp
     qwt_transform.cpp
     qwt_wheel.cpp
     qwt_widget_overlay.cpp
)

add_definitions(-DQWT_MAKEDLL)

add_library(qwt SHARED ${QWT_SOURCES})
target_link_libraries(qwt Qt5::Widgets Qt5::Concurrent Qt5::PrintSupport Qt5::Svg Qt5::OpenGL)
set_target_properties(qwt PROPERTIES
                      AUTOMOC On
                      VERSION 6.1.6)

target_include_directories(qwt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
install_library (TARGETS qwt)
