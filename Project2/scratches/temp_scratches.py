class CollapsibleBox(QGroupBox):
    toggled = pyqtSignal(QGroupBox)

    def __init__(self, title="", df_getter=None, parent=None):
        super(CollapsibleBox, self).__init__(title, parent)
        self.df_getter = df_getter
        self.content_widget = None
        self.toggle_button = QToolButton(self)
        self.toggle_button.setText(str(self.title()))
        self.toggle_button.setCheckable(True)
        self.toggle_button.setChecked(True)
        self.toggle_button.setArrowType(Qt.DownArrow if self.toggle_button.isChecked() else Qt.RightArrow)
        self.toggle_button.clicked.connect(self.on_pressed)

        self.content_area = QVBoxLayout()
        self.setLayout(self.content_area)

        self.animation = QPropertyAnimation(self, b"maximumHeight")
        self.animation.setDuration(300)
        self.animation.setStartValue(0)
        self.animation.setEndValue(100)  # Adjust to the height of your content
        self.animation.setEasingCurve(QEasingCurve.InOutQuart)
        self.animation.finished.connect(self.on_animation_finished)

    def add_content(self, widget):
        if self.content_widget is not None:
            # Remove the old content widget if it exists
            self.content_area.removeWidget(self.content_widget)
            self.content_widget.deleteLater()

        self.content_widget = widget  # Assign the new content widget
        self.content_area.addWidget(widget)

    def toggle(self):
        show = self.toggle_button.isChecked()
        print(f"Toggle button checked: {show}")
        if show:
            if self.content_widget is None:
                df = self.df_getter()
                self.content_widget = QTableView()
                model = DataFrameModel(df)
                self.content_widget.setModel(model)
                self.add_content(self.content_widget)
            self.content_widget.setVisible(True)
            self.update_animation_end_value()
        else:
            if self.content_widget is not None:
                self.content_widget.setVisible(False)
                self.content_area.removeWidget(self.content_widget)
                self.content_widget.deleteLater()
                self.content_widget = None
        self.setCollapsed(not show)


    def setCollapsed(self, collapsed):
        print(f"Collapsing: {collapsed}, current end value: {self.animation.endValue()}")
        if collapsed:
            self.animation.setEndValue(0)
        else:
            self.update_animation_end_value()
        self.animation.start()

    def update_animation_end_value(self):
        if self.content_widget:
            if self.content_widget.isVisible():
                content_height = self.content_widget.sizeHint().height()
            else:
                self.content_widget.setMaximumHeight(0)
                content_height = 0
            print(f"New end value: {content_height}")
            self.animation.setEndValue(content_height)
        else:
            print("update_animation_end_value called with no content widget")

    def isExpanded(self):
        return self.toggle_button.isChecked()

    def on_pressed(self):
        checked = self.toggle_button.isChecked()
        self.toggle_button.setArrowType(Qt.DownArrow if checked else Qt.RightArrow)
        self.toggle()

    def on_animation_finished(self):
        if not self.toggle_button.isChecked():
            # After the animation finished, if the box is collapsed, remove the content
            if self.content_widget is not None:
                self.content_widget.setVisible(False)
                self.content_area.removeWidget(self.content_widget)
                self.content_widget.deleteLater()
                self.content_widget = None
            self.toggled.emit(self)  # Now emit the toggled signal
    # def on_pressed(self):
    #     try:
    #         checked = self.toggle_button.isChecked()
    #         self.toggle_button.setArrowType(Qt.DownArrow if checked else Qt.RightArrow)
    #         self.toggle()
    #         # Emit the signal after the animation has finished
    #         self.animation.finished.connect(lambda: self.toggled.emit(self))
    #     except Exception as e:
    #         print(f"An error occurred: {e}")
    #         # Optionally use QErrorMessage to show the error in a dialog
    #         error_dialog = QErrorMessage(self)
    #         error_dialog.showMessage(str(e))


class MultipleDataFrameView(QWidget):
    def __init__(self, data_frame_getters, names):
        super().__init__()
        layout = QVBoxLayout(self)
        scroll_area = QScrollArea()
        scroll_widget = QWidget()
        scroll_layout = QVBoxLayout(scroll_widget)
        self.boxes = []

        for get_df, name in zip(data_frame_getters, names):
            collapsible_box = CollapsibleBox(title=name, df_getter=get_df)
            collapsible_box.toggled.connect(self.toggle_boxes)
            self.boxes.append(collapsible_box)
            scroll_layout.addWidget(collapsible_box)

        scroll_widget.setLayout(scroll_layout)
        scroll_area.setWidget(scroll_widget)
        scroll_area.setWidgetResizable(True)
        layout.addWidget(scroll_area)
        self.setLayout(layout)

    def toggle_boxes(self, toggled_box):
        try:
            # Close all boxes except the one that was toggled
            for box in self.boxes:
                if box is not toggled_box:
                    box.setCollapsed(True)
                else:
                    # Only load and show the DataFrame if the box is being expanded
                    if not box.isExpanded():
                        df = toggled_box.df_getter()  # Retrieve the DataFrame
                        view = QTableView()
                        model = DataFrameModel(df)
                        view.setModel(model)
                        view.setStyleSheet("""
                            QTableView {
                                selection-background-color: #E0E0E0;
                                font: 11pt "Calibri";
                            }
                            QHeaderView::section {
                                background-color: #F0F0F0;
                                padding: 4px;
                                border: 1px solid #D0D0D0;
                                font-size: 10pt;
                            }
                        """)
                        toggled_box.add_content(view)  # Add the QTableView as content
                    # Toggle the state of the box
                    box.setCollapsed(not box.isExpanded())
        except Exception as e:
            print(f"An error occurred: {e}")
            error_dialog = QErrorMessage(self)
            error_dialog.showMessage(str(e))


# class MultipleDataFrameView(QWidget):
#     def __init__(self, data_frames, names):
#         super().__init__()
#         layout = QVBoxLayout(self)
#         scroll_area = QScrollArea()
#         scroll_widget = QWidget()
#         scroll_layout = QVBoxLayout(scroll_widget)
#
#         for df, name in zip(data_frames, names):
#             collapsible_box = CollapsibleBox(title=name)
#             view = QTableView()
#             model = DataFrameModel(df)
#             view.setModel(model)
#             view.setStyleSheet("""
#                 QTableView {
#                     selection-background-color: #E0E0E0;
#                     font: 11pt "Calibri";
#                 }
#                 QHeaderView::section {
#                     background-color: #F0F0F0;
#                     padding: 4px;
#                     border: 1px solid #D0D0D0;
#                     font-size: 10pt;
#                 }
#             """)
#
#             collapsible_box.add_content(view)
#             scroll_layout.addWidget(collapsible_box)
#
#         scroll_widget.setLayout(scroll_layout)
#         scroll_area.setWidget(scroll_widget)
#         scroll_area.setWidgetResizable(True)
#         layout.addWidget(scroll_area)
#         self.setLayout(layout)

# class MultipleDataFrameView(QWidget):
#     def __init__(self, data_frame_getters, names):
#         super().__init__()
#         layout = QVBoxLayout(self)
#         scroll_area = QScrollArea()
#         scroll_widget = QWidget()
#         scroll_layout = QVBoxLayout(scroll_widget)
#         self.boxes = []
#
#         for get_df, name in zip(data_frame_getters, names):
#             collapsible_box = CollapsibleBox(title=name, df_getter=get_df)  # Pass the df_getter here
#             collapsible_box.toggled.connect(self.toggle_boxes)
#             self.boxes.append(collapsible_box)
#             scroll_layout.addWidget(collapsible_box)
#
#         scroll_widget.setLayout(scroll_layout)
#         scroll_area.setWidget(scroll_widget)
#         scroll_area.setWidgetResizable(True)
#         layout.addWidget(scroll_area)
#         self.setLayout(layout)
#
#     def toggle_boxes(self, toggled_box):
#         try:
#             for box in self.boxes:
#                 if box is not toggled_box:
#                     box.toggle_button.setChecked(False)
#                     box.toggle()
#                 else:
#                     box.toggle_button.setChecked(True)
#                     box.toggle()  # This will handle showing or hiding the content
#                     if not box.isExpanded():
#                         df = toggled_box.df_getter()
#                         view = QTableView()
#                         model = DataFrameModel(df)
#                         view.setModel(model)
#                         view.setStyleSheet("""
#                             QTableView {
#                                 selection-background-color: #E0E0E0;
#                                 font: 11pt "Calibri";
#                             }
#                             QHeaderView::section {
#                                 background-color: #F0F0F0;
#                                 padding: 4px;
#                                 border: 1px solid #D0D0D0;
#                                 font-size: 10pt;
#                             }
#                         """)
#                         toggled_box.add_content(view)
#                         box.setCollapsed(False)
#         except Exception as e:
#             print(f"An error occurred: {e}")
#             error_dialog = QErrorMessage(self)
#             error_dialog.showMessage(str(e))

# def toggle_boxes(self, toggled_box):
#     try:
#         for box in self.boxes:
#             if box is not toggled_box:
#                 box.setCollapsed(True)
#             else:
#                 if not box.isExpanded():
#                     df = toggled_box.df_getter()  # Get the DataFrame using the getter function
#                     view = QTableView()
#                     model = DataFrameModel(df)
#                     view.setModel(model)
#                     view.setStyleSheet("""
#                         QTableView {
#                             selection-background-color: #E0E0E0;
#                             font: 11pt "Calibri";
#                         }
#                         QHeaderView::section {
#                             background-color: #F0F0F0;
#                             padding: 4px;
#                             border: 1px solid #D0D0D0;
#                             font-size: 10pt;
#                         }
#                     """)
#                     toggled_box.add_content(view)
#                     box.setCollapsed(False)  # Expand the current box
#     except Exception as e:
#         print(f"An error occurred: {e}")
#         error_dialog = QErrorMessage(self)
#         error_dialog.showMessage(str(e))
# def setCollapsed(self, collapsed):
#     if collapsed:
#         self.animation.setDirection(QPropertyAnimation.Backward)
#         self.animation.setEndValue(0)  # Ensure it collapses back to a minimal height
#     else:
#         self.update_animation_end_value()  # Make sure endValue is up to date
#         self.animation.setDirection(QPropertyAnimation.Forward)
#     self.animation.start()


# def add_content(self, widget):
#     self.content_area.addWidget(widget)
#     self.content_area.addStretch()


# if __name__ == "__main__":
#     app = QApplication(sys.argv)
#
#     # Sample data frames and names
#     data_frame_getters = [lambda df=item[0]: df for item in res_1.values()]
#     names = [re.compile(r"\d+").sub("", _) for _ in res_1.keys()]
#
#     multi_df_view = MultipleDataFrameView(data_frame_getters, names)
#     multi_df_view.resize(800, 600)
#     multi_df_view.show()
#
#     sys.exit(app.exec_())

"""ABCDEFGHI"""

# from PyQt5.QtWidgets import QApplication, QTableView, QSplitter, QVBoxLayout, QWidget
# from PyQt5.QtCore import Qt
# import sys
#
#
# class MultipleDataFrameView(QWidget):
#     def __init__(self, data_frames):
#         super().__init__()
#         layout = QVBoxLayout(self)
#
#         # Create a QSplitter widget and set its orientation to horizontal
#         splitter = QSplitter(Qt.Horizontal)
#
#         # Iterate over the data frames and add each as a QTableView to the splitter
#         for df in data_frames:
#             view = QTableView()
#             view.setStyleSheet("""
#                 QTableView {
#                     selection-background-color: #E0E0E0;
#                     font: 11pt "Calibri";
#                 }
#                 QHeaderView::section {
#                     background-color: #F0F0F0;
#                     padding: 4px;
#                     border: 1px solid #D0D0D0;
#                     font-size: 10pt;
#                 }
#             """)
#             model = DataFrameModel(df)
#             view.setModel(model)
#             splitter.addWidget(view)
#
#         layout.addWidget(splitter)
#         self.setLayout(layout)
#
#
# if __name__ == "__main__":
#     app = QApplication(sys.argv)
#
#     # Sample data frames
#     df1 = res_1["Aggregate 1"][0]
#     df2 = res_1["Hash Join 10"][0]
#
#     # Pass the list of data frames to the custom widget
#     multi_df_view = MultipleDataFrameView([df1, df2])
#     multi_df_view.resize(800, 600)
#     multi_df_view.show()
#
#     sys.exit(app.exec_())

# # %%
# from PyQt5.QtWidgets import QApplication, QTableView
# import sys
#
# if __name__ == "__main__":
#     app = QApplication(sys.argv)
#
#     # Ensure 'res_1' is defined and 'res_1["Aggregate 1"][0]' is a DataFrame
#     df = pd.DataFrame(res_1["Aggregate 1"][0])
#
#     model = DataFrameModel(df)
#     view = QTableView()
#     view.setModel(model)
#     view.resize(800, 600)
#     view.show()
#     sys.exit(app.exec_())