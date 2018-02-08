/** 
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
package com.altibase.altimon.properties;

public enum ThresholdOperator {
    GT(">") {
        @Override
        public boolean apply(double x1, double x2) {
            return (x1 > x2);
        }
    },
    LT("<") {
        @Override
        public boolean apply(double x1, double x2) {
            return (x1 < x2);
        }
    },
    GE(">=") {
        @Override
        public boolean apply(double x1, double x2) {
            return (x1 >= x2);
        }
    },
    LE("<=") {
        @Override
        public boolean apply(double x1, double x2) {
            return (x1 <= x2);
        }
    },
    EQ("==") {
        @Override
        public boolean apply(double x1, double x2) {
            return (x1 == x2);
        }
    },
    NE("!=") {
        @Override
        public boolean apply(double x1, double x2) {
            return (x1 != x2);
        }
    },
    NONE("") {
        @Override
        public boolean apply(double x1, double x2) {
            return false;
        }
    };

    private final String symbol;

    private ThresholdOperator(String symbol) {
        this.symbol = symbol;		
    }

    public abstract boolean apply(double x1, double x2);

    public static ThresholdOperator getOperatorBySymbol(String op) {
        for (ThresholdOperator tOp : ThresholdOperator.values()) {
            if (tOp.symbol.equals(op)) {
                return tOp;
            }
        }
        throw new IllegalArgumentException("Illegal operator : " + op);
    }
}
