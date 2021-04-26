#include "c_componentelemview.h"
#include "ui_c_componentelemview.h"

c_componentElemView::c_componentElemView(c_component *_component, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::c_componentElemView), component(_component) {
    ui->setupUi(this);
    ui->unitComboBox->insertItems(0,recipe::unitToString.values());
    ui->ingredientLineEdit->setText(component->getIngredient().getName());
    ui->quantityUnitLabel->setText(QString("%1%2").arg(component->getQuantity()).arg(recipe::unitToString[component->getUnit()]));
    ui->unitComboBox->setCurrentText(recipe::unitToString[component->getUnit()]);
    ui->quantitySpinBox->setValue(double(component->getQuantity()));

    QObject::connect(ui->deleteButton,&QPushButton::clicked, [=] () {
        emit deleteMe();
    });

    this->setFixedHeight(23);

    switchMode();
}

c_componentElemView::~c_componentElemView() {
    delete ui;
}

QList<QPropertyAnimation *> c_componentElemView::switchMode(int mode) {
    QList<QPropertyAnimation *> res;
    QFontMetrics metrics(ui->ingredientLineEdit->font());
    int widthTotal = 0;
    switch (mode) {
        case recipe::modes::display:
        case recipe::modes::resume:
            ui->unitComboBox->hide();
            ui->quantitySpinBox->hide();
            ui->deleteButton->hide();
            ui->checkBox->show();
            ui->quantityUnitLabel->show();
            ui->ingredientLineEdit->setReadOnly(true);
            ui->ingredientLineEdit->setStyleSheet("QLineEdit {"
                                                  "  border : 1px solid white;"
                                                  "  background: transparent;"
                                                  "}");
            widthTotal = metrics.horizontalAdvance(ui->quantityUnitLabel->text()+ui->ingredientLineEdit->text()) + ui->checkBox->width() + static_cast<QHBoxLayout*>(this->layout())->spacing()*2 + 10;
            this->setFixedWidth(widthTotal);
            break;
        case recipe::modes::edition:
            ui->unitComboBox->show();
            ui->quantitySpinBox->show();
            ui->deleteButton->show();
            ui->checkBox->hide();
            ui->quantityUnitLabel->hide();
            ui->ingredientLineEdit->setReadOnly(false);
            ui->ingredientLineEdit->setStyleSheet("");
            widthTotal = metrics.horizontalAdvance(ui->quantityUnitLabel->text()+ui->ingredientLineEdit->text()) + ui->checkBox->width() + static_cast<QHBoxLayout*>(this->layout())->spacing()*2 + 10;
            this->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
            this->setFixedHeight(23);
            break;
    default:
        break;
    }
    return res;
}

c_component *c_componentElemView::getComponent() {
    return component;
}

void c_componentElemView::setFocus() {
    ui->ingredientLineEdit->setFocus();
    ui->ingredientLineEdit->clear();
}
