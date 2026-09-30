// SPDX-FileCopyrightText: 2023-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 2.15

Item {
    id: root
    required property bool useColumnLayout
    property alias model: listView.model
    property alias delegate: listView.delegate
    property alias spacing: listView.spacing
    property alias count: listView.count
    property alias add: listView.add
    property alias remove: listView.remove
    property alias move: listView.move
    property alias displaced: listView.displaced
    property alias addDisplaced: listView.addDisplaced
    property alias removeDisplaced: listView.removeDisplaced
    property alias moveDisplaced: listView.moveDisplaced
    ListView {
        id: listView
        anchors.fill: parent
        orientation: useColumnLayout ? ListView.Vertical : ListView.Horizontal
        layoutDirection: Qt.LeftToRight
        verticalLayoutDirection: ListView.TopToBottom
        interactive: false
        Accessible.role: Accessible.List
        Component.onCompleted: {
            Accessible.id = "ListView"
            Qt.callLater(root.updateImplicitSizes)
        }
        onCountChanged: Qt.callLater(root.updateImplicitSizes)
        onSpacingChanged: Qt.callLater(root.updateImplicitSizes)
    }

    Connections {
        target: listView.contentItem
        function onChildrenChanged() { Qt.callLater(root.updateImplicitSizes) }
    }

    function calculateImplicitWidth(prev, current) {
        if (useColumnLayout) {
            return Math.max(prev, current)
        } else {
            if (prev == 0) {
                return current
            }
            return prev + root.spacing + current
        }
    }

    function calculateImplicitHeight(prev, current) {
        if (useColumnLayout) {
            if (prev == 0) {
                return current
            }
            return prev + root.spacing + current
        } else {
            return Math.max(prev, current)
        }
    }

    function indexAt(x, y) {
        return listView.indexAt(x, y)
    }

    // Use imperative property updates instead of direct bindings to avoid a
    // binding loop: implicitHeight -> visibleChildren -> layout -> implicitHeight.
    // Qt.callLater coalesces multiple signals into a single deferred update,
    // breaking the synchronous re-evaluation cycle.
    property real _implicitContentWidth: 1
    property real _implicitContentHeight: 1

    function updateImplicitSizes() {
        let width = 0
        let height = 0
        for (let child of listView.contentItem.visibleChildren) {
            width = calculateImplicitWidth(width, child.implicitWidth)
            height = calculateImplicitHeight(height, child.implicitHeight)
        }
        // TODO: above qt6.8 implicitSize to 0 will make size to 0 default.
        // so make minimum implicitSize to 1, find why and remove below
        _implicitContentWidth = Math.max(width, 1)
        _implicitContentHeight = Math.max(height, 1)
    }

    implicitWidth: _implicitContentWidth
    implicitHeight: _implicitContentHeight
}
