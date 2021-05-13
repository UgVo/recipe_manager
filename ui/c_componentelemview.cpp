#include "c_componentelemview.h"
#include "ui_c_componentelemview.h"
#include "utils/c_dbmanager.h"

int c_componentElemView::heigthWidget = 23;

c_componentElemView::c_componentElemView(c_component *_component, c_widget *widget, QWidget *parent) :
    c_widget(parent,widget),
    ui(new Ui::c_componentElemView), component(_component) {
    ui->setupUi(this);
    ui->unitComboBox->insertItems(0,recipe::unitToString.values());
    ui->unitComboBox->setFixedHeight(heigthWidget);
    ui->ingredientLineEdit->setText(component->getIngredient().getName());
    ui->ingredientLineEdit->setFixedHeight(heigthWidget);
    ui->quantityUnitLabel->setText(QString("%1%2").arg(component->getQuantity()).arg(recipe::unitToString[component->getUnit()]));
    ui->quantityUnitLabel->setFixedHeight(heigthWidget);
    ui->unitComboBox->setCurrentText(recipe::unitToString[component->getUnit()]);
    ui->unitComboBox->setFixedHeight(heigthWidget);
    ui->quantitySpinBox->setValue(component->getQuantity());
    ui->quantitySpinBox->setFixedHeight(heigthWidget);
    ui->checkBox->setFixedHeight(heigthWidget);
    ui->deleteButton->show();

    QObject::connect(ui->deleteButton,&QPushButton::clicked, [=] () {
        emit deleteMe();
    });

    this->setFixedHeight(heigthWidget);

    switchMode();
}

c_componentElemView::~c_componentElemView() {
    delete ui;
}

QAbstractAnimation *c_componentElemView::switchMode(modes target, bool animated, int time) {
    QParallelAnimationGroup *res = new QParallelAnimationGroup();
    QFontMetrics metrics(ui->ingredientLineEdit->font());
    QPoint pos(0,0);
    QSize targetSizeLabel;
    switch (target) {
        case modes::display:
        case modes::resume:
        case modes::minimal:
            ui->quantityUnitLabel->setFixedWidth(metrics.horizontalAdvance(ui->quantityUnitLabel->text())+4);
            ui->checkBox->show();
            ui->quantityUnitLabel->show();
            ui->ingredientLineEdit->setReadOnly(true);
            ui->ingredientLineEdit->setStyleSheet("QLineEdit {"
                                                  "  border : 1px solid white;"
                                                  "  background: transparent;"
                                                  "}");

            if (animated) {
                res->addAnimation(targetPositionAnimation(ui->checkBox,pos,time/3));
                pos += QPoint(ui->checkBox->width() + insideBorder,0);
                res->addAnimation(targetPositionAnimation(ui->quantitySpinBox,QPoint(0,-ui->quantitySpinBox->height()),time/3));
                res->addAnimation(targetPositionAnimation(ui->quantityUnitLabel,pos,time/3));
                res->addAnimation(targetPositionAnimation(ui->unitComboBox,QPoint(ui->quantitySpinBox->width() + insideBorder,-ui->quantitySpinBox->height()),time/3));
                pos += QPoint(ui->quantityUnitLabel->width() + insideBorder,0);
                res->addAnimation(targetGeometryAnimation(ui->ingredientLineEdit,QSize(metrics.horizontalAdvance(ui->ingredientLineEdit->text())+10,heigthWidget),pos,time));
                res->addAnimation(targetSizeAnimation(this,getSize(target),time));
                res->addAnimation(targetPositionAnimation(ui->deleteButton,QPoint(getSize(modes::edition).width()+ui->deleteButton->width(),0),time/3));
            } else {
                ui->checkBox->move(pos);
                pos += QPoint(ui->checkBox->width() + insideBorder,0);
                ui->quantitySpinBox->move(QPoint(0,-ui->quantitySpinBox->height()));
                ui->quantityUnitLabel->move(pos);
                ui->unitComboBox->move(QPoint(ui->quantitySpinBox->width() + insideBorder,-ui->quantitySpinBox->height()));
                pos += QPoint(ui->quantityUnitLabel->width() + insideBorder,0);
                ui->ingredientLineEdit->move(pos);
                ui->ingredientLineEdit->setFixedSize(QSize(metrics.horizontalAdvance(ui->ingredientLineEdit->text())+6,heigthWidget));
                this->setFixedSize(getSize(target));
                ui->deleteButton->move(QPoint(getSize(modes::edition).width()+ui->deleteButton->width(),0));
            }

            break;
        case modes::edition:
            ui->ingredientLineEdit->setReadOnly(false);
            ui->ingredientLineEdit->setStyleSheet("");
            targetSizeLabel = QSize(getSize(target).width() - ui->unitComboBox->width() - ui->quantitySpinBox->width() - insideBorder*3 - ui->deleteButton->width(),heigthWidget);
            if (mode != target) {
                ui->deleteButton->move(QPoint(getSize(target).width(),0));
            }

            if (animated) {
                res->addAnimation(targetPositionAnimation(ui->checkBox,QPoint(0,-ui->checkBox->height()),time + time/3,time));
                res->addAnimation(targetPositionAnimation(ui->quantitySpinBox,pos,time + time/3,time));
                res->addAnimation(targetPositionAnimation(ui->quantityUnitLabel,QPoint(ui->checkBox->width() + insideBorder,ui->quantityUnitLabel->height()),time + time/3,time));
                pos += QPoint(ui->quantitySpinBox->width()+insideBorder,0);
                res->addAnimation(targetPositionAnimation(ui->unitComboBox,pos,time + time/3,time));
                pos += QPoint(ui->unitComboBox->width() + insideBorder,0);
                res->addAnimation(targetGeometryAnimation(ui->ingredientLineEdit,targetSizeLabel,pos,time));
                res->addAnimation(targetSizeAnimation(this,getSize(target),time));
                res->addAnimation(targetPositionAnimation(ui->deleteButton,QPoint(getSize(target).width()-ui->deleteButton->width(),0),time + time/3,time));
            } else {
                ui->checkBox->move(QPoint(0,ui->checkBox->height()));
                ui->quantitySpinBox->move(pos);
                ui->quantityUnitLabel->move(pos + QPoint(0,ui->quantityUnitLabel->height()));
                pos += QPoint(ui->quantitySpinBox->width()+insideBorder,0);
                ui->unitComboBox->move(pos);
                pos += QPoint(ui->unitComboBox->width() + insideBorder,0);
                ui->ingredientLineEdit->move(pos);
                ui->ingredientLineEdit->setFixedSize(targetSizeLabel);
                this->setFixedSize(getSize(target));
                ui->deleteButton->move(QPoint(getSize(target).width()-ui->deleteButton->width(),0));
            }
            break;
    default:
        break;
    }
    mode = target;
    return res;
}

QSize c_componentElemView::getSize(modes target) const {
    QSize res;
    QFontMetrics metrics(ui->ingredientLineEdit->font());
    switch (target) {
        case modes::display:
        case modes::resume:
        case modes::minimal:
            res.setWidth(ui->quantityUnitLabel->width()
                         + ui->checkBox->width()
                         + ui->ingredientLineEdit->width()
                         + insideBorder*2);
            res.setHeight(heigthWidget);
            break;
        case modes::edition:
            res.setWidth(m_parent->getWidth(target) - 2*insideBorder);
            res.setHeight(heigthWidget);
            break;
        default:
            break;
    }
    return res;
}

void c_componentElemView::save() {
    QString ingredientName = ui->ingredientLineEdit->text();
    if (ingredientName.isEmpty() || ui->quantitySpinBox->value()==0) {
        emit deleteMe();
    } else {
        QList<c_ingredient> ingredients = c_dbManager::getIngredients().values();
        c_ingredient ingre;
        for (int i = 0; i < ingredients.size(); ++i) {
            if (!ingredientName.compare(ingredients[i].getName())) {
                if (ingre.isEmpty()) {
                    ingre = ingredients[i];
                }
                if (ingredients[i].getSubRecipe().isEmpty()) {
                    ingre = ingredients[i];
                    break;
                }
            }
        }
        if (ingre.isEmpty()) {
            ingre.setName(ingredientName);
        }

        component->setIngredient(ingre);
        component->setQuantity(ui->quantitySpinBox->value());
        component->setUnit(recipe::unitToString.key(ui->unitComboBox->currentText()));

        ui->ingredientLineEdit->setText(component->getIngredient().getName());
        if (recipe::unitToString[component->getUnit()].size() > 2) {
            ui->quantityUnitLabel->setText(QString("%1 %2%3").arg(component->getQuantity()).arg(recipe::unitToString[component->getUnit()]).arg(component->getQuantity()>1?"s":""));
        } else {
            ui->quantityUnitLabel->setText(QString("%1%2").arg(component->getQuantity()).arg(recipe::unitToString[component->getUnit()]));
        }
    }
}

void c_componentElemView::rollback() {
    ui->ingredientLineEdit->setText(component->getIngredient().getName());
    ui->quantityUnitLabel->setText(QString("%1%2").arg(component->getQuantity()).arg(recipe::unitToString[component->getUnit()]));
    ui->unitComboBox->setCurrentText(recipe::unitToString[component->getUnit()]);
    ui->quantitySpinBox->setValue(component->getQuantity());
}

c_component *c_componentElemView::getComponent() {
    return component;
}

void c_componentElemView::setFocus() {
    ui->ingredientLineEdit->setFocus();
    ui->ingredientLineEdit->clear();
}
