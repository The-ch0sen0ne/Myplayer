#include "myslider.h"
#include <QMouseEvent>
#include <QDebug>
#include <QStyle>
MySlider::MySlider(QWidget *parent) : QSlider(parent)
{

}

//关注滑块点击事件，使得滑块快速移动
void MySlider::mousePressEvent(QMouseEvent *ev)
{
//    int value = minimum()+(ev->pos().x()*1.0/width())*(maximum()-minimum());
    //
    int value = QStyle::sliderValueFromPosition(minimum(),maximum(),ev->pos().x()*1.0,width());
    setValue(value);
    QSlider::mousePressEvent(ev);

    emit clicked(this);

}
