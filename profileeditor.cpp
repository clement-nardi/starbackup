#include "profileeditor.h"
#include "ui_profileeditor.h"
#include "checkablefoldertree.h"
#include "onlycheckedfoldertree.h"
#include "previewfoldertree.h"

ProfileEditor::ProfileEditor(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::ProfileEditor)
{
    ui->setupUi(this);
    CheckableFolderTree *model = new CheckableFolderTree(this);
    OnlyCheckedFolderTree *previewModel = new OnlyCheckedFolderTree(model,ui->Destination,this);
    PreviewFolderTree * preview = new PreviewFolderTree(model,this);
    ui->SourceFoldersTree->setModel(model);
    ui->PreviewTree->setItemsExpandable(false);
    ui->PreviewTree->setModel(previewModel);
    ui->PreviewTree->expandAll();
}

ProfileEditor::~ProfileEditor()
{
    delete ui;
}

void ProfileEditor::changeEvent(QEvent *e)
{
    QTabWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
