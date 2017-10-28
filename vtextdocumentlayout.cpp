#include "vtextdocumentlayout.h"

#include <QTextDocument>
#include <QTextBlock>
#include <QTextFrame>
#include <QTextLayout>
#include <QPointF>
#include <QFontMetrics>
#include <QFont>
#include <QDebug>


VTextDocumentLayout::VTextDocumentLayout(QTextDocument *p_doc)
    : QAbstractTextDocumentLayout(p_doc),
      m_pageWidth(0),
      m_margin(p_doc->documentMargin()),
      m_width(0),
      m_maximumWidthBlockNumber(-1),
      m_height(0),
      m_blockCount(0)
{
}

void VTextDocumentLayout::draw(QPainter *p_painter, const PaintContext &p_context)
{
    qDebug() << "VTextDocumentLayout draw()" << p_context.clip << p_context.cursorPosition << p_context.selections.size();

}

int VTextDocumentLayout::hitTest(const QPointF &p_point, Qt::HitTestAccuracy p_accuracy) const
{
    qDebug() << "VTextDocumentLayout hitTest()" << p_point;
    return -1;
}

int VTextDocumentLayout::pageCount() const
{
    return 1;
}

QSizeF VTextDocumentLayout::documentSize() const
{
    return QSizeF(m_width, m_height);
}

QRectF VTextDocumentLayout::frameBoundingRect(QTextFrame *p_frame) const
{
    return QRectF(0, 0, qMax(m_pageWidth, m_width), qreal(INT_MAX));
}

QRectF VTextDocumentLayout::blockBoundingRect(const QTextBlock &p_block) const
{
    if (!p_block.isValid()) {
        return QRectF();
    }

    const BlockInfo &info = m_blocks[p_block.blockNumber()];
    qDebug() << "blockBoundingRect()" << p_block.blockNumber()
             << info.m_offset << info.m_rect;

    Q_ASSERT(!info.m_rect.isNull());

    return info.m_rect;
}

void VTextDocumentLayout::documentChanged(int p_from, int p_oldLength, int p_length)
{
    QTextDocument *doc = document();
    int newBlockCount = doc->blockCount();

    // Update the margin.
    m_margin = doc->documentMargin();

    int charsChanged = p_oldLength + p_length;

    // TODO: optimize the way to find out affected blocks.
    QTextBlock changeStartBlock = doc->findBlock(p_from);
    QTextBlock changeEndBlock = doc->findBlock(qMax(0, p_from + charsChanged));

    qDebug() << "documentChanged" << p_from << p_oldLength << p_length
             << changeStartBlock.blockNumber() << changeEndBlock.blockNumber() << newBlockCount;

    bool needRelayout = false;
    if (changeStartBlock == changeEndBlock
        && newBlockCount == m_blockCount) {
        // Change single block internal only.
        QTextBlock block = changeStartBlock;
        if (block.isValid() && block.length()) {
            QRectF oldBr = blockBoundingRect(block);
            layoutBlock(block);
            QRectF newBr = blockBoundingRect(block);
            // Only one block is affected.
            if (newBr.height() == oldBr.height()) {
                emit updateBlock(block);
                return;
            }
        }
    } else {
        // Clear layout of all affected blocks.
        QTextBlock block = changeStartBlock;
        do {
            clearBlockLayout(block);
            if (block == changeEndBlock) {
                break;
            }

            block = block.next();
        } while(block.isValid());

        needRelayout = true;
    }

    updateBlockCount(newBlockCount);

    if (needRelayout) {
        // Relayout all affected blocks.
        QTextBlock block = changeStartBlock;
        do {
            layoutBlock(block);
            if (block == changeEndBlock) {
                break;
            }

            block = block.next();
        } while(block.isValid());
    }

    updateDocumentSize();

    emit update(QRectF(0., -m_margin, 1000000000., 1000000000.));
}

void VTextDocumentLayout::clearBlockLayout(QTextBlock &p_block)
{
    qDebug() << "clearBlockLayout()" << p_block.blockNumber();
    p_block.clearLayout();
    int num = p_block.blockNumber();
    if (num < m_blocks.size()) {
        m_blocks[num].reset();
        clearOffsetFrom(num + 1);
    }
}

void VTextDocumentLayout::clearOffsetFrom(int p_blockNumber)
{
    for (int i = p_blockNumber; i < m_blocks.size(); ++i) {
        if (m_blocks[i].m_offset < 0) {
            Q_ASSERT(validateBlocks());
            break;
        }

        m_blocks[i].m_offset = -1;
    }
}

void VTextDocumentLayout::fillOffsetFrom(int p_blockNumber)
{
    qreal offset = m_blocks[p_blockNumber].m_offset;
    Q_ASSERT(offset >= 0);
    offset += m_blocks[p_blockNumber].m_rect.height();
    for (int i = p_blockNumber + 1; i < m_blocks.size(); ++i) {
        BlockInfo &info = m_blocks[i];
        if (info.m_valid && !info.m_rect.isNull()) {
            info.m_offset = offset;
            offset += info.m_rect.height();
        } else {
            break;
        }
    }
}

bool VTextDocumentLayout::validateBlocks() const
{
    bool valid = true;
    for (int i = 0; i < m_blocks.size(); ++i) {
        const BlockInfo &info = m_blocks[i];
        if (!info.m_valid) {
            continue;
        }

        if (info.m_offset < 0) {
            valid = false;
        } else {
            if (info.m_rect.isNull()
                || !valid) {
                return false;
            }
        }
    }

    return true;
}

void VTextDocumentLayout::updateBlockCount(int p_count)
{
    if (m_blockCount != p_count) {
        m_blockCount = p_count;
        m_blocks.resize(m_blockCount);
    }
}

void VTextDocumentLayout::layoutBlock(const QTextBlock &p_block)
{
    qDebug() << "layoutBlock()" << p_block.blockNumber();
    QTextDocument *doc = document();

    // The height (y) of the next line.
    qreal height = 0;
    QTextLayout *tl = p_block.layout();
    QTextOption option = doc->defaultTextOption();
    tl->setTextOption(option);

    int extraMargin = 0;
    if (option.flags() & QTextOption::AddSpaceForLineAndParagraphSeparators) {
        QFontMetrics fm(p_block.charFormat().font());
        extraMargin += fm.width(QChar(0x21B5));
    }

    qreal availableWidth = m_pageWidth;
    if (availableWidth <= 0) {
        availableWidth = qreal(INT_MAX);
    }

    availableWidth -= 2 * m_margin + extraMargin;

    tl->beginLayout();

    while (true) {
        QTextLine line = tl->createLine();
        if (!line.isValid()) {
            break;
        }

        line.setLeadingIncluded(true);
        line.setLineWidth(availableWidth);
        line.setPosition(QPointF(m_margin, height));
        height += line.height();
    }

    tl->endLayout();

    // Set this block's line count to the its layout's line count.
    // That is one block may occupy multiple visual lines.
    const_cast<QTextBlock&>(p_block).setLineCount(p_block.isVisible() ? tl->lineCount() : 0);

    // Update the info about this block.
    finishBlockLayout(p_block);
}

void VTextDocumentLayout::finishBlockLayout(const QTextBlock &p_block)
{
    // Update rect and offset.
    int num = p_block.blockNumber();
    Q_ASSERT(m_blocks.size() > num);

    BlockInfo &info = m_blocks[num];
    info.reset();

    if (p_block.isValid() && p_block.isVisible()) {
        QTextLayout *tl = p_block.layout();
        Q_ASSERT(tl->lineCount());
        QRectF br = QRectF(QPointF(0, 0), tl->boundingRect().bottomRight());
        if (tl->lineCount() == 1) {
            br.setWidth(qMax(br.width(), tl->lineAt(0).naturalTextWidth()));
        }

        br.adjust(0, 0, m_margin, 0);
        if (!p_block.next().isValid()) {
            br.adjust(0, 0, 0, m_margin);
        }

        info.m_valid = true;
        info.m_rect = br;

        int pre = previousValidBlockNumber(num);
        qDebug() << "finishBlockLayout" << num << pre << br << (pre > -1 ? m_blocks[pre].m_offset : -1);
        if (pre == -1) {
            info.m_offset = 0;
        } else if (m_blocks[pre].m_offset >= 0) {
            info.m_offset = m_blocks[pre].m_offset + m_blocks[pre].m_rect.height();
            Q_ASSERT(info.m_offset >= 0);
        }

        if (info.m_offset >= 0) {
            fillOffsetFrom(num);
        }
    } else {
        info.m_valid = false;
    }
}

int VTextDocumentLayout::nextValidBlockNumber(int p_number) const
{
    for (int i = p_number + 1; i < m_blocks.size(); ++i) {
        if (m_blocks[i].m_valid) {
            return i;
        }
    }

    return -1;
}

int VTextDocumentLayout::previousValidBlockNumber(int p_number) const
{
    for (int i = p_number - 1; i > -1; --i) {
        if (m_blocks[i].m_valid) {
            return i;
        }
    }

    return -1;
}

qreal VTextDocumentLayout::blockWidth(const QTextBlock &p_block) const
{
    int num = p_block.blockNumber();
    if (num < m_blocks.size()
        && !m_blocks[num].m_rect.isNull()) {
        return m_margin + m_blocks[num].m_rect.width();
    }

    return 0;
}

void VTextDocumentLayout::updateDocumentSize()
{
    int idx = previousValidBlockNumber(m_blocks.size());
    Q_ASSERT(idx > -1);
    if (m_blocks[idx].m_offset >= 0) {
        int oldHeight = m_height;
        int oldWidth = m_width;

        m_height = m_blocks[idx].m_offset + m_blocks[idx].m_rect.height();

        m_width = 0;
        for (int i = 0; i < m_blocks.size(); ++i) {
            const BlockInfo &info = m_blocks[i];
            if (!info.m_valid) {
                continue;
            }

            Q_ASSERT(!info.m_rect.isNull());
            m_width = qMax(m_width, info.m_rect.width());
        }

        m_width += m_margin;

        if (oldHeight != m_height
            || oldWidth != m_width) {
            emit documentSizeChanged(documentSize());
        }
    }
}
