#pragma once

#include <QtWidgets/QTreeWidget>
#include <QtCore/QHash>
#include <QtCore/QSettings>
#include <QtWidgets/QWidget>
#include <QtGui/QIcon>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>

#include "mainWindow.h"
#include "../pluginManager/proxyEditorManager.h"
#include "../../qrkernel/ids.h"

namespace  qReal{
namespace gui{

/// Class for representing tree with elements of the editors.
class PaletteTree: public QWidget
{
Q_OBJECT
public:
	explicit PaletteTree(QWidget *parent = 0);

	/// Adds item type to some editor's tree.
	/// @param id Item id.
	/// @param name Item name.
	/// @param description Item description.
	/// @param icon Item icon.
	/// @param tree Editor's tree.
	/// @param parent Parent of item's group.
	void addItemType(Id const &id, QString const &name, QString const &description
			, QIcon const &icon, QSize const &preferredSize
			, QTreeWidget *tree, QTreeWidgetItem *parent);

	/// Adds top item type to some editor's tree.
	/// @param id Item id.
	/// @param name Item name.
	/// @param description Item description.
	/// @param icon Item icon.
	/// @param tree Editor's tree.
	void addTopItemType(Id const &id, QString const &name, QString const &description
			, QIcon const &icon, QSize const &preferredSize, QTreeWidget *tree);

	/** Adds all editor's elements to appropriate tree.
	  @param editorManager Editor manager which all editors with elements are taken from.
	  @param editor Editor
	  @param diagram Diagram that corresponds to chosen editor.
	*/
	void addEditorElements(EditorManagerInterface &editorManagerProxy, const Id &editor, const Id &diagram);

	/// Initialize connection editor's combobox with slot that sets active editor.
	void initDone();

	/** Delete chosen editor.
	  @param id Editor id.
	*/
	void deleteEditor(Id const &id);
	QComboBox* comboBox() const;
	QVector<QString> editorsNames() const;
	Id currentEditor() const;

	/// Set item with such id as active in ComboBox.
	void setComboBox(Id const &id);
	void setIconsView(bool iconsView);
	bool iconsView() const;
	void setItemsCountInARow(int count);
	int itemsCountInARow() const;

	/// Set item with such index as active in ComboBox.
	void setComboBoxIndex(int index);

	/// Set saved item index as current in ComboBox.
	void setComboBoxIndex();

	/// Saves selected editor and expanded groups into settings
	void saveConfiguration();

	/** Load palette and set some representation.
	  @param isIconsView This variable corresponds to representation.
	  @param itemsCount Items count in a row.
	  @param editorManager Editor manager which all editors with elements are taken from.
	*/
	void loadPalette(bool isIconsView, int itemsCount, EditorManagerInterface *editorManagerProxy);
	~PaletteTree();
	void initMainWindow(MainWindow *mainWindow);

signals:
	void paletteParametersChanged();

public slots:
	/// Change expanded/collapsed state of current tree.
	void changeExpansionState();

	void setActiveEditor(int index);
	void setActiveEditor(Id id);

	/// Recreate PaletteTree.
	void recreateTrees();

	/// Changes widget representation.
	void changeRepresentation();

private:
	/// Change icon and tooltip
	void setExpansionButtonAppearance();

	/// Collapse all nodes of current tree.
	void collapse();

	/// Expand all nodes of current tree.
	void expand();

	/// Returns maximum count of items in all rows of widget
	int maxItemsCountInARow() const;

	virtual void resizeEvent(QResizeEvent *);

	/// Change icon's sizes in widget
	void resizeIcons();

	/// Forbids to make copies of the object.
	explicit PaletteTree(const PaletteTree &paletteTree);

	/** Method-comparator for sorting Ids by displayed name.
	Needs EditorManager instance to work,
	but qSort() prohibits it to be a member of an object.
	So making it static does the trick.
	*/
	static bool idLessThan(const Id &s1, const Id &s2);

	/** Recursive procedure that collapses node with his children.
	  @param item Node which will be collapsed with all his children.
	*/
	void collapseChildren(QTreeWidgetItem *item);

	/** Recursive procedure that expands node with his children.
	  @param item Node which will be expanded with all his children.
	*/
	void expandChildren(QTreeWidgetItem *item);

	/// Expands all children of given tree widget
	void expand(QTreeWidget const *tree);

	/// Creates all PaletteTree widgets.
	void createPaletteTree();

	/// Deletes all PaletteTree widgets.
	void deletePaletteTree();

	/// Adds group of editor's elements to appropriate tree to some top element.
	/// @param tmpList List with sorted group elements.
	/// @param editorTree Editor's tree
	/// @param item Editor's tree node for adding in it tmpList.
	void addItemsRow(IdList const &tmpIdList, QTreeWidget *editorTree, QTreeWidgetItem *item);

	/// Fills palette tree by editors.
	/// @param editorManager Editor manager which all editors with elements are taken from.
	void loadEditors(EditorManagerInterface &editorManagerProxy);

	/// EditorManager instance used to sort palette's content.
	/// Made static to be used inside idLessThan()
	static EditorManagerInterface *mEditorManager;

	MainWindow *mMainWindow;

	/// Hash table with editor ids.
	QHash<Id, int> mCategories;

	/// Pointer to current tree.
	QTreeWidget *mTree;

	/// Button that changes expansion state of current tree
	QToolButton *mChangeExpansionState;

	/// Button that changes palette representation.
	QToolButton *mChangeRepresentation;

	/// Vector with all editor's trees.
	QVector <QTreeWidget *> mEditorsTrees;

	/// Vector with all editor's names.
	QVector <QString> mEditorsNames;

	/// Combobox with editors.
	QComboBox *mComboBox;

	/// Main layout of the widget.
	QVBoxLayout *mLayout;

	/// Current editor number.
	int mCurrentEditor;

	/// Representation flag
	bool mIconsView;

	/// Whether expand/collapse button expands tree
	bool mNodesStateButtonExpands;

	/// Count of items in a row in icon's representation
	int mItemsCountInARow;
};
}
}
