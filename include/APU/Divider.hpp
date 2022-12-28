#ifndef TIMER_H
#define TIMER_H

/**
 * @todo write docs
 */
class Divider
{
public:
    void reset(); // does NOT output a clock
    void setReloadValue(int value);

    void clock();
    bool isHit();
private:
    bool m_hit = false;

    int m_counter = 0;
    int m_reload = 0;
};

#endif // TIMER_H
