#include "messagedelegate.h"
#include <QPainter>
#include <QFontMetrics>
#include <QPen>
#include <QBrush>
#include <QDebug>
#include <QApplication>
#include <QDateTime>
#include <QFile>

MessageDelegate::MessageDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &option,const QModelIndex &index) const
{
    // 获取消息内容
    QString text = index.data(ContentRole).toString();
    // 获取字体
    QFont font = option.font;
    font.setPointSize(10); // 与绘制时的字体大小一致
    QFontMetrics fm(font);

    // 计算气泡大小
    int maxWidth = 280; // 稍小一点
    QSize bubbleSize = calculateBubbleSize(text, font, maxWidth);

    // 减小间距
    int avatarHeight = 35; // 与绘制的头像大小一致

    // 计算总高度 - 减小间隔
    int totalHeight = bubbleSize.height() + 8; // 气泡高度 + 很小的上下间距

    // 确保至少包含头像高度
    if (totalHeight < avatarHeight + 4) {
        totalHeight = avatarHeight + 4;
    }

    // 如果有时间戳，额外增加高度（但可以很小）
    QDateTime timestamp = index.data(TimestampRole).toDateTime();
    if (timestamp.isValid()) {
        totalHeight += 2; // 改为很小的额外高度
    }
    return QSize(option.rect.width(), totalHeight);
}

void MessageDelegate::paint(QPainter *painter,const QStyleOptionViewItem &option,const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    // 获取消息数据
    QString text = index.data(ContentRole).toString();  // 使用角色枚举
    QString sender = index.data(SenderRole).toString();
    QDateTime timestamp = index.data(TimestampRole).toDateTime();
    bool isMyMessage = index.data(IsMyMessageRole).toBool();
    QString avatar = index.data(AvatarRole).toString();

    QString timeStr = timestamp.toString("hh:mm");

    // 绘制背景
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, QColor(240, 240, 240));
    } else {
        painter->fillRect(option.rect, QColor(250, 250, 250));
    }

    // 计算布局
    int avatarSize = 35;
    int padding = 5; // 减小为5px
    int maxBubbleWidth = 300;

    // 计算气泡尺寸
    QSize bubbleSize = calculateBubbleSize(text, option.font, maxBubbleWidth);

    // 布局矩形
    QRect avatarRect;
    QRect bubbleRect;
    QRect timeRect;

    if (isMyMessage) {
        // 我的消息在右侧
        int avatarX = option.rect.right() - avatarSize - padding;
        int avatarY = option.rect.top() + padding;
        avatarRect = QRect(avatarX, avatarY, avatarSize, avatarSize);

        int bubbleX = avatarX - bubbleSize.width() - 10;
        int bubbleY = option.rect.top() + padding;
        bubbleRect = QRect(bubbleX, bubbleY, bubbleSize.width(), bubbleSize.height());

        int timeX = bubbleRect.left() - 50;
        int timeY = bubbleRect.top() + 5;
        timeRect = QRect(timeX, timeY, 45, 20);

    } else {
        // 对方消息在左侧
        int avatarX = option.rect.left() + padding;
        int avatarY = option.rect.top() + padding;
        avatarRect = QRect(avatarX, avatarY, avatarSize, avatarSize);

        int bubbleX = avatarRect.right() + 10;
        int bubbleY = option.rect.top() + padding;
        bubbleRect = QRect(bubbleX, bubbleY, bubbleSize.width(), bubbleSize.height());

        int timeX = bubbleRect.right() + 5;
        int timeY = bubbleRect.top() + 5;
        timeRect = QRect(timeX, timeY, 45, 20);
    }

    // 绘制头像
    drawAvatar(painter, avatarRect, avatar, sender);

    // 绘制消息气泡
    bool isSelected = option.state & QStyle::State_Selected;
    drawMessageBubble(painter, bubbleRect, isMyMessage, isSelected);

    // 绘制消息文本
    drawMessageText(painter, bubbleRect, text, option.font);

    // 绘制时间
    drawMessageTime(painter, timeRect, timeStr, isMyMessage);

    // 绘制分割线
    painter->setPen(QColor(240, 240, 240));
    painter->drawLine(option.rect.left(), option.rect.bottom() - 1,
                      option.rect.right(), option.rect.bottom() - 1);

    painter->restore();
}

QSize MessageDelegate::calculateBubbleSize(const QString &text,
                                           const QFont &font,
                                           int maxWidth) const
{
    QFontMetrics fm(font);

    // 计算文本需要的宽度
    int textWidth = fm.horizontalAdvance(text);

    // 限制最大宽度
    int bubbleWidth = qMin(textWidth + 40, maxWidth);

    // 计算文本高度
    int lineHeight = fm.lineSpacing();
    int lines = 1;

    if (textWidth > maxWidth - 40) {
        // 需要换行
        QStringList words = text.split(' ');
        int currentLineWidth = 0;
        lines = 1;

        for (const QString &word : words) {
            int wordWidth = fm.horizontalAdvance(word + " ");
            if (currentLineWidth + wordWidth > maxWidth - 40) {
                lines++;
                currentLineWidth = wordWidth;
            } else {
                currentLineWidth += wordWidth;
            }
        }
    }

    int bubbleHeight = lines * lineHeight + 20;  // 文本高度 + 上下内边距

    return QSize(bubbleWidth, bubbleHeight);
}

void MessageDelegate::drawMessageBubble(QPainter *painter,
                                        const QRect &rect,
                                        bool isMyMessage,
                                        bool isSelected) const
{
    painter->save();

    // 气泡颜色
    QColor bubbleColor;
    if (isMyMessage) {
        bubbleColor = isSelected ? QColor(158, 234, 106) : QColor(149, 236, 105);
    } else {
        bubbleColor = isSelected ? QColor(240, 240, 240) : QColor(255, 255, 255);
    }

    // 边框颜色
    QColor borderColor = isSelected ? QColor(200, 200, 200) : QColor(225, 225, 225);

    // 绘制圆角矩形气泡
    painter->setPen(QPen(borderColor, 1));
    painter->setBrush(QBrush(bubbleColor));
    painter->drawRoundedRect(rect, 8, 8);

    // 绘制小三角
    QPolygon triangle;
    if (isMyMessage) {
        // 右侧气泡的小三角在右边
        triangle << QPoint(rect.right() + 1, rect.top() + 10)
                 << QPoint(rect.right() - 5, rect.top() + 5)
                 << QPoint(rect.right() - 5, rect.top() + 15);
    } else {
        // 左侧气泡的小三角在左边
        triangle << QPoint(rect.left() - 1, rect.top() + 10)
                 << QPoint(rect.left() + 5, rect.top() + 5)
                 << QPoint(rect.left() + 5, rect.top() + 15);
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(bubbleColor));
    painter->drawPolygon(triangle);

    painter->restore();
}

void MessageDelegate::drawMessageText(QPainter *painter,
                                      const QRect &rect,
                                      const QString &text,
                                      const QFont &font) const
{
    painter->save();

    // 设置字体
    QFont messageFont = font;
    messageFont.setPointSize(10);
    painter->setFont(messageFont);

    // 设置文本颜色
    painter->setPen(QColor(0, 0, 0));

    // 计算内边距
    int padding = 8;
    QRect textRect = rect.adjusted(padding, padding, -padding, -padding);

    // 绘制文本
    painter->drawText(textRect, Qt::TextWordWrap, text);

    painter->restore();
}

void MessageDelegate::drawMessageTime(QPainter *painter,
                                      const QRect &rect,
                                      const QString &time,
                                      bool isMyMessage) const
{
    painter->save();

    // 设置时间文本样式
    QFont timeFont = painter->font();
    timeFont.setPointSize(8);
    painter->setFont(timeFont);

    painter->setPen(QColor(150, 150, 150));

    // 对齐方式
    int flags = Qt::AlignVCenter;
    if (isMyMessage) {
        flags |= Qt::AlignRight;
    } else {
        flags |= Qt::AlignLeft;
    }

    painter->drawText(rect, flags, time);

    painter->restore();
}

void MessageDelegate::drawAvatar(QPainter *painter,
                                 const QRect &rect,
                                 const QString &avatarPath,
                                 const QString &sender) const
{
    painter->save();

    // 绘制头像圆形背景
    painter->setPen(Qt::NoPen);

    // 生成头像背景色（根据用户名）
    uint hash = qHash(sender);
    QColor bgColor = QColor::fromHsv(hash % 360, 150, 230);
    painter->setBrush(bgColor);

    // 绘制圆形
    painter->drawEllipse(rect);

    // 如果有头像图片，绘制图片
    if (!avatarPath.isEmpty() && QFile::exists(avatarPath)) {
        QPixmap avatar(avatarPath);
        avatar = avatar.scaled(rect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // 创建圆形蒙版
        QPixmap circularPixmap(rect.size());
        circularPixmap.fill(Qt::transparent);

        QPainter pixmapPainter(&circularPixmap);
        pixmapPainter.setRenderHint(QPainter::Antialiasing);
        pixmapPainter.setBrush(Qt::black);
        pixmapPainter.drawEllipse(0, 0, rect.width(), rect.height());
        pixmapPainter.end();

        // 应用蒙版
        avatar.setMask(circularPixmap.mask());
        painter->drawPixmap(rect, avatar);
    } else {
        // 绘制用户首字母
        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 12, QFont::Bold));

        QString initial = sender.left(1).toUpper();
        painter->drawText(rect, Qt::AlignCenter, initial);
    }

    painter->restore();
}
