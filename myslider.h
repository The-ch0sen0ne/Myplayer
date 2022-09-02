#ifndef MYSLIDER_H
#define MYSLIDER_H

#include <QSlider>

class MySlider : public QSlider
{
    Q_OBJECT
public:
    explicit MySlider(QWidget *parent = nullptr);

signals:
    void clicked(MySlider *slider);


private:
    void mousePressEvent(QMouseEvent *ev) override;
};

#endif // MYSLIDER_H
